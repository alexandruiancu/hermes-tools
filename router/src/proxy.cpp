#include "proxy.hpp"
#include "metrics.hpp"
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>

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

// Find matching backend for a request path
EndpointConfig* ProxyHandler::find_backend(const std::string& path) {
    // First check API routes for exact matches
    for (const auto& [service, routes] : api_routes_) {
        for (const auto& route : routes) {
            if (path == route || path.rfind(route, 0) == 0) {
                // Find the corresponding endpoint
                for (auto& ep : endpoints_) {
                    if (ep.name == service && ep.enabled) {
                        return &ep;
                    }
                }
            }
        }
    }
    
    // Fall back to path_prefix matching
    for (auto& ep : endpoints_) {
        if (ep.enabled && path.rfind(ep.path_prefix, 0) == 0) {
            return &ep;
        }
    }
    
    return nullptr;
}

void ProxyHandler::handle_request(const http::request<http::string_body>& req,
                                 http::response<http::string_body>& res) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Log request
    std::cout << "Request: " << req.method_string() << " " << req.target() << std::endl;
    
    // Handle local routes (not forwarded to backend)
    std::string path(req.target());
    
    // Handle /v1/models - forward to llama-cpp backend
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
    
    EndpointConfig* backend = find_backend(path);
    
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

void ProxyHandler::forward_request(const http::request<http::string_body>& req,
                                 EndpointConfig& backend,
                                 http::response<http::string_body>& res) {
    // This is a placeholder for the actual forwarding logic.
    // In a real implementation, this would open a connection to the backend
    // and proxy the request/response.
    res.result(http::status::ok);
    res.body() = "Forwarded to " + backend.name + " (" + backend.host + ":" + std::to_string(backend.port) + ")";
    res.prepare_payload();
}

// Handle local routes (not forwarded to backend)
void ProxyHandler::handle_local_metrics(const http::request<http::string_body>& req,
                                      http::response<http::string_body>& res) {
    // Collect metrics data
    auto request_counts = Metrics::get_request_counts();
    auto avg_latencies = Metrics::get_avg_latency();
    
    // Build JSON response
    std::string body = "{\n  \"metrics\": {\n";
    
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
        auto it2 = request_counts.find(service);
        if (it2 != std::prev(request_counts.end())) {
            body += ",";
        }
        body += "\n";
    }
    
    body += "  }\n}";
    
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    res.body() = body;
    res.prepare_payload();
}

// Handle /v1/models - forward to llama-cpp backend
void ProxyHandler::handle_v1_models(const http::request<http::string_body>& req,
                                   http::response<http::string_body>& res) {
    // Find llama-cpp backend (look for endpoint with "llama" in name or path)
    EndpointConfig* llama_backend = nullptr;
    for (auto& ep : endpoints_) {
        if (ep.enabled && (ep.name.find("llama") != std::string::npos || 
                           ep.path_prefix.find("llama") != std::string::npos)) {
            llama_backend = &ep;
            break;
        }
    }
    
    // If no llama-cpp backend found, return a default models list
    if (!llama_backend) {
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        std::string body = R"({
  "object": "list",
  "data": [
    {
      "object": "model",
      "id": "llama-model",
      "created": 1700000000,
      "owned_by": "organization"
    }
  ]
})";
        res.body() = body;
        res.prepare_payload();
        return;
    }
    
    // Forward request to llama-cpp backend
    std::cout << "[PROXY] Forwarding /v1/models to llama-cpp backend: " 
              << llama_backend->host << ":" << llama_backend->port << std::endl;
    
    // Create a new request to the backend
    http::request<http::string_body> backend_req{http::method::get, req.base()};
    backend_req.target() = "/v1/models";
    backend_req.set(http::field::host, llama_backend->host);
    backend_req.set(http::field::accept, "application/json");
    
    // Forward any relevant headers
    if (req.is_request(http::field::authorization)) {
        backend_req.set(http::field::authorization, req[http::field::authorization]);
    }
    
    // Use async HTTP client to forward the request
    // For simplicity, we'll return a placeholder response here
    // In production, you'd use boost::asio::ip::tcp::socket to connect and forward
    
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    std::string body = R"({
  "object": "list",
  "data": [
    {
      "object": "model",
      "id": "llama-model",
      "created": 1700000000,
      "owned_by": "organization"
    }
  ]
})";
    res.body() = body;
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
