#include "proxy.hpp"
#include "metrics.hpp"
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <openssl/ssl.h>
#include <iostream>
#include <cstdlib>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

ProxyHandler::ProxyHandler() {
    // Load configuration from external file
    if (ConfigLoader::is_loaded()) {
        endpoints_ = ConfigLoader::get_endpoints();
        api_routes_ = ConfigLoader::get_api_routes();
        std::cout << "ProxyHandler initialized with " << endpoints_.size() << " endpoints" << std::endl;
    } else {
        std::cerr << "Warning: Configuration not loaded, proxy may not function correctly" << std::endl;
    }
}

// Find matching backend for a request path, optionally by model name
EndpointConfig* ProxyHandler::find_backend(const std::string& path, const std::string& model_name) {
    // If model name is provided, extract endpoint from it
    if (!model_name.empty()) {
        auto [ep_name, _] = parse_model_name(model_name);
        if (!ep_name.empty()) {
            for (auto& ep : endpoints_) {
                if (ep.name == ep_name && ep.enabled) {
                    return &ep;
                }
            }
        }
    }
    
    // Fall back to API route matching
    for (const auto& [service, routes] : api_routes_) {
        for (const auto& route : routes) {
            if (path == route || path.rfind(route, 0) == 0) {
                for (auto& ep : endpoints_) {
                    if (ep.name == service && ep.enabled) {
                        return &ep;
                    }
                }
            }
        }
    }
    
    return nullptr;
}

// Parse model name in "endpoint:model" format
std::pair<std::string, std::string> ProxyHandler::parse_model_name(const std::string& model) {
    size_t colon = model.rfind(':');
    if (colon == std::string::npos || colon == 0 || colon == model.length() - 1) {
        return {"", model};
    }
    return {model.substr(0, colon), model.substr(colon + 1)};
}

// Extract a string field from a simple JSON body (no nesting)
std::string ProxyHandler::extract_json_string(const std::string& body, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = body.find(search);
    if (pos == std::string::npos) return "";
    
    pos = body.find('"', pos + search.length());
    if (pos == std::string::npos) return "";
    pos++;
    
    size_t end = body.find('"', pos);
    if (end == std::string::npos) return "";
    
    return body.substr(pos, end - pos);
}

void ProxyHandler::handle_request(const http::request<http::string_body>& req,
                                 http::response<http::string_body>& res) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Log request
    std::cout << "Request: " << req.method_string() << " " << req.target() << std::endl;
    
    // Handle local routes (not forwarded to backend)
    std::string path(req.target());
    
    // Handle /v1/models - aggregate from all backends
    if (path == "/v1/models") {
        handle_v1_models(req, res);
        return;
    }
    
    // Handle /v1/metrics/{model_name} - per-model metrics
    if (path.rfind("/v1/metrics/", 0) == 0) {
        std::string model_name = path.substr(12); // Extract model name after "/v1/metrics/"
        if (!model_name.empty()) {
            handle_v1_metrics_model(model_name, res);
            return;
        }
    }
    
    if (path == "/v1/metrics") {
        handle_local_metrics(req, res);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        Metrics::record_latency("local", duration.count());
        return;
    }
    
    // For POST requests to model endpoints, extract model name from body for routing
    std::string model_name;
    if (req.method() == http::verb::post) {
        model_name = extract_json_string(req.body(), "model");
    }
    
    EndpointConfig* backend = find_backend(path, model_name);
    
    if (!backend) {
        res.result(http::status::not_found);
        res.body() = "Backend not found for path: " + path;
        res.prepare_payload();
        return;
    }
    
    // Record metrics
    Metrics::record_request(backend->name, req.method_string());
    
    // Forward request
    forward_request(req, *backend, res);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    Metrics::record_latency(backend->name, duration.count());
}

// Actually forward an HTTP request to a backend and read the response
bool ProxyHandler::http_forward(const std::string& method, const std::string& target,
                               const std::string& body, const std::string& content_type,
                               EndpointConfig& backend,
                               http::response<http::string_body>& res) {
    try {
        net::io_context ioc{1};
        
        // Resolve endpoint
        net::ip::tcp::resolver resolver{ioc};
        auto results = resolver.resolve(backend.host, std::to_string(backend.port));
        
        // Build the HTTP request (shared between SSL and non-SSL paths)
        http::request<http::string_body> backend_req;
        backend_req.method_string(method);
        backend_req.target(target);
        backend_req.set(http::field::host, backend.host);
        backend_req.set(http::field::accept, "*/*");
        backend_req.set(http::field::connection, "close");
        
        if (!content_type.empty()) {
            backend_req.set(http::field::content_type, content_type);
        }
        
        if (!body.empty()) {
            backend_req.body() = body;
        }
        
        // Inject API key if configured for this backend
        if (!backend.api_key.empty()) {
            backend_req.set(http::field::authorization, "Bearer " + backend.api_key);
        }
        
        backend_req.prepare_payload();
        
        beast::error_code ec;
        
        if (backend.ssl) {
            // --- SSL/TLS path ---
            net::ip::tcp::socket raw_socket{ioc};
            net::connect(raw_socket, results);
            
            // Create SSL context for client
            net::ssl::context ssl_ctx(net::ssl::context::tlsv12_client);
            ssl_ctx.set_verify_mode(net::ssl::verify_none);
            
            // Wrap socket in SSL stream (stream takes ownership of the socket)
            net::ssl::stream<net::ip::tcp::socket> ssl_socket(std::move(raw_socket), ssl_ctx);
            
            // Enable SNI so the server selects the correct certificate
            if (!SSL_set_tlsext_host_name(ssl_socket.native_handle(), backend.host.c_str())) {
                std::cerr << "[PROXY] Failed to set SNI for " << backend.host << ": "
                          << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
                return false;
            }
            
            // Perform SSL handshake as client
            ssl_socket.handshake(net::ssl::stream_base::client);
            
            // Send request over SSL
            http::write(ssl_socket, backend_req, ec);
            if (ec) {
                std::cerr << "[PROXY] SSL write error to " << backend.name << ": " << ec.message() << std::endl;
                return false;
            }
            
            // Read response over SSL
            beast::flat_buffer buffer;
            http::read(ssl_socket, buffer, res, ec);
            if (ec) {
                std::cerr << "[PROXY] SSL read error from " << backend.name << ": " << ec.message() << std::endl;
                return false;
            }
        } else {
            // --- Plain TCP path ---
            net::ip::tcp::socket socket{ioc};
            net::connect(socket, results);
            
            // Send request
            http::write(socket, backend_req, ec);
            if (ec) {
                std::cerr << "[PROXY] Write error to " << backend.name << ": " << ec.message() << std::endl;
                return false;
            }
            
            // Read response
            beast::flat_buffer buffer;
            http::read(socket, buffer, res, ec);
            if (ec) {
                std::cerr << "[PROXY] Read error from " << backend.name << ": " << ec.message() << std::endl;
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[PROXY] Exception forwarding to " << backend.name << ": " << e.what() << std::endl;
        return false;
    }
}

void ProxyHandler::forward_request(const http::request<http::string_body>& req,
                                 EndpointConfig& backend,
                                 http::response<http::string_body>& res) {
    // Determine the target path and body
    std::string target = req.target();
    std::string body = req.body();
    std::string content_type;
    if (req.contains(http::field::content_type)) {
        content_type = req[http::field::content_type];
    }
    
    // If the request has a model name with endpoint prefix, strip it from the body
    if (!body.empty() && req.method() == http::verb::post) {
        std::string model = extract_json_string(body, "model");
        if (!model.empty()) {
            auto [ep_name, clean_model] = parse_model_name(model);
            if (!ep_name.empty() && !clean_model.empty()) {
                // Replace "endpoint:model" with just "model" in the body
                std::string old_str = "\"model\":\"" + model + "\"";
                std::string new_str = "\"model\":\"" + clean_model + "\"";
                size_t pos = body.find(old_str);
                if (pos != std::string::npos) {
                    body = body.substr(0, pos) + new_str + body.substr(pos + old_str.length());
                }
            }
        }
    }
    
    bool success = http_forward(
        req.method_string(), target, body, content_type, backend, res
    );
    
    if (!success) {
        res.result(http::status::bad_gateway);
        res.body() = "Backend error: " + backend.name;
        res.prepare_payload();
    }
}

// Handle local routes (not forwarded to backend)
void ProxyHandler::handle_local_metrics(const http::request<http::string_body>& req,
                                      http::response<http::string_body>& res) {
    // Collect metrics data
    auto request_counts = Metrics::get_request_counts();
    auto avg_latencies = Metrics::get_avg_latency();
    
    // Build JSON response
    std::string body = "{\n  \"metrics\": {\n";
    
    size_t service_count = 0;
    for (const auto& [service, count] : request_counts) {
        body += "    \"" + service + "\": {\n";
        body += "      \"request_count\": " + std::to_string(count) + ",\n";
        auto it = avg_latencies.find(service);
        if (it != avg_latencies.end()) {
            body += "      \"avg_latency_ms\": " + std::to_string(it->second) + "\n";
        } else {
            body += "      \"avg_latency_ms\": 0\n";
        }
        body += "    }";
        
        // Add comma if not last entry
        if (service_count < request_counts.size() - 1) {
            body += ",";
        }
        body += "\n";
        service_count++;
    }
    
    body += "  }\n}";
    
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    res.body() = body;
    res.prepare_payload();
}

// Handle /v1/models - aggregate from all backends that list this route
void ProxyHandler::handle_v1_models(const http::request<http::string_body>& req,
                                   http::response<http::string_body>& res) {
    std::cout << "[PROXY] Aggregating /v1/models from all backends" << std::endl;
    
    // Collect all models from each backend that has "/v1/models" in its routes
    std::string merged_data = "";
    bool first = true;
    
    for (auto& ep : endpoints_) {
        if (!ep.enabled) continue;
        
        // Check if this endpoint has /v1/models in its api_routes
        bool has_models_route = false;
        auto it = api_routes_.find(ep.name);
        if (it != api_routes_.end()) {
            for (const auto& route : it->second) {
                if (route == "/v1/models") {
                    has_models_route = true;
                    break;
                }
            }
        }
        if (!has_models_route) continue;
        
        // Forward request to this backend
        http::response<http::string_body> backend_res;
        bool ok = http_forward("GET", "/v1/models", "", "", ep, backend_res);
        
        if (!ok) {
            std::cerr << "[PROXY] Failed to get models from " << ep.name << ", skipping" << std::endl;
            continue;
        }
        
        // Parse the response body to extract model entries
        // Simple approach: find the "data" array and extract objects
        std::string backend_body = backend_res.body();
        size_t data_start = backend_body.find("\"data\":[");
        if (data_start == std::string::npos) {
            data_start = backend_body.find("\"data\": [");
        }
        if (data_start == std::string::npos) continue;
        
        // Find the opening bracket
        data_start = backend_body.find('[', data_start);
        if (data_start == std::string::npos) continue;
        
        // Find matching closing bracket
        int depth = 0;
        size_t data_end = data_start;
        for (size_t i = data_start; i < backend_body.length(); i++) {
            if (backend_body[i] == '[') depth++;
            if (backend_body[i] == ']') depth--;
            if (depth == 0) {
                data_end = i;
                break;
            }
        }
        
        std::string models_array = backend_body.substr(data_start + 1, data_end - data_start - 1);
        if (models_array.empty()) continue;
        
        // Split by "{" and process each model object
        size_t obj_pos = 0;
        while ((obj_pos = models_array.find('{', obj_pos)) != std::string::npos) {
            depth = 0;
            size_t obj_end = obj_pos;
            for (size_t i = obj_pos; i < models_array.length(); i++) {
                if (models_array[i] == '{') depth++;
                if (models_array[i] == '}') depth--;
                if (depth == 0) {
                    obj_end = i + 1;
                    break;
                }
            }
            
            std::string model_obj = models_array.substr(obj_pos, obj_end - obj_pos);
            
            // Prefix the model "id" field with "endpoint:"
            std::string id_search = "\"id\":\"";
            size_t id_pos = model_obj.find(id_search);
            if (id_pos != std::string::npos) {
                id_pos += id_search.length();
                std::string prefix = ep.name + ":";
                model_obj = model_obj.substr(0, id_pos) + prefix + model_obj.substr(id_pos);
            }
            
            if (!first) {
                merged_data += ",";
            }
            merged_data += model_obj;
            first = false;
            
            obj_pos = obj_end;
        }
    }
    
    // Build the merged response
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    
    if (merged_data.empty()) {
        // No backends available, return empty list
        res.body() = R"({"object":"list","data":[]})";
    } else {
        res.body() = "{\"object\":\"list\",\"data\":[" + merged_data + "]}";
    }
    
    res.prepare_payload();
}

// Handle /v1/metrics/{model_name} - per-model metrics
void ProxyHandler::handle_v1_metrics_model(const std::string& model_name,
                                          http::response<http::string_body>& res) {
    auto model_metrics = Metrics::get_model_metrics(model_name);
    
    // Build JSON response
    std::string body = "{\n";
    body += "  \"model_name\": \"" + model_name + "\",\n";
    body += "  \"loaded\": " + std::string(model_metrics.loaded ? "true" : "false") + ",\n";
    body += "  \"request_count\": " + std::to_string(model_metrics.request_count) + ",\n";
    
    // Calculate average latency
    double avg_latency = 0;
    if (!model_metrics.latencies.empty()) {
        double sum = 0;
        for (double l : model_metrics.latencies) sum += l;
        avg_latency = sum / model_metrics.latencies.size();
    }
    body += "  \"avg_latency_ms\": " + std::to_string(avg_latency) + ",\n";
    
    // Token counts
    body += "  \"token_counts\": {\n";
    body += "    \"input\": " + std::to_string(model_metrics.token_counts.first) + ",\n";
    body += "    \"output\": " + std::to_string(model_metrics.token_counts.second) + "\n";
    body += "  }\n";
    body += "}";
    
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    res.body() = body;
    res.prepare_payload();
}
