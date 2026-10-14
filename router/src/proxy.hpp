#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <vector>
#include <map>

#include "config_loader.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

class ProxyHandler {
public:
    ProxyHandler();
    
    void handle_request(const http::request<http::string_body>& req, 
                       http::response<http::string_body>& res);
    
private:
    // Find matching backend for a request path
    EndpointConfig* find_backend(const std::string& path);
    
    // Check if path matches any API route
    std::string find_api_route(const std::string& path);
    
    // Handle local routes (not forwarded to backend)
    void handle_local_metrics(const http::request<http::string_body>& req,
                             http::response<http::string_body>& res);
    
    void forward_request(const http::request<http::string_body>& req, 
                        EndpointConfig& backend,
                        http::response<http::string_body>& res);
    
    // Handle /v1/models endpoint - forward to llama-cpp
    void handle_v1_models(const http::request<http::string_body>& req,
                         http::response<http::string_body>& res);
    
    // Handle /v1/metrics/{model_name} endpoint
    void handle_v1_metrics_model(const std::string& model_name,
                                http::response<http::string_body>& res);
    
    // Configuration loaded from external file
    std::vector<EndpointConfig> endpoints_;
    std::map<std::string, std::vector<std::string>> api_routes_;
};
