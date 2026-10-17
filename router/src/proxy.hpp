#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <vector>
#include <map>
#include <utility>

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
    // Find matching backend for a request path, optionally by model name
    EndpointConfig* find_backend(const std::string& path, const std::string& model_name = "");
    
    // Parse model name in "endpoint:model" format → (endpoint_name, clean_model_name)
    std::pair<std::string, std::string> parse_model_name(const std::string& model);
    
    // Extract a string field from a JSON body (simple manual parsing)
    std::string extract_json_string(const std::string& body, const std::string& key);
    
    // Handle local routes (not forwarded to backend)
    void handle_local_metrics(const http::request<http::string_body>& req,
                             http::response<http::string_body>& res);
    
    void forward_request(const http::request<http::string_body>& req, 
                        EndpointConfig& backend,
                        http::response<http::string_body>& res);
    
    // Actually forward an HTTP request to a backend and read the response
    bool http_forward(const std::string& method, const std::string& target,
                     const std::string& body, const std::string& content_type,
                     EndpointConfig& backend,
                     http::response<http::string_body>& res);
    
    // Handle /v1/models endpoint - aggregate from all backends
    void handle_v1_models(const http::request<http::string_body>& req,
                         http::response<http::string_body>& res);
    
    // Handle /v1/metrics/{model_name} endpoint
    void handle_v1_metrics_model(const std::string& model_name,
                                http::response<http::string_body>& res);
    
    // Configuration loaded from external file
    std::vector<EndpointConfig> endpoints_;
    std::map<std::string, std::vector<std::string>> api_routes_;
};
