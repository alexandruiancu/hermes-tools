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
    // Initialize backends
    backends_.push_back({"llama-cpp", "op.local", 11500, "/llama"});
    backends_.push_back({"firecrawl", "op.local", 3001, "/firecrawl"});
    backends_.push_back({"image-gen", "op.local", 11903, "/image"});
    backends_.push_back({"searxng", "op.local", 0, "/searx"}); // TBD
    backends_.push_back({"openrouter", "openrouter.ai", 443, "/openrouter"});
}

BackendConfig* ProxyHandler::find_backend(const std::string& path) {
    for (auto& backend : backends_) {
        if (path.rfind(backend.path_prefix, 0) == 0) {
            return &backend;
        }
    }
    return nullptr;
}

void ProxyHandler::handle_request(const http::request<http::string_body>& req,
                                 http::response<http::string_body>& res) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Log request
    std::cout << "Request: " << req.method_string() << " " << req.target() << std::endl;
    
    BackendConfig* backend = find_backend(std::string(req.target()));
    
    if (!backend) {
        res.result(http::status::not_found);
        res.body() = "Backend not found";
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
                                 BackendConfig& backend,
                                 http::response<http::string_body>& res) {
    // This is a placeholder for the actual forwarding logic.
    // In a real implementation, this would open a connection to the backend
    // and proxy the request/response.
    res.result(http::status::ok);
    res.body() = "Forwarded to " + backend.name;
    res.prepare_payload();
}
