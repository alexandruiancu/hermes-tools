#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

struct BackendConfig {
    std::string name;
    std::string host;
    unsigned short port;
    std::string path_prefix;
};

class ProxyHandler {
public:
    ProxyHandler();
    
    void handle_request(const http::request<http::string_body>& req, 
                       http::response<http::string_body>& res);
    
private:
    std::vector<BackendConfig> backends_;
    
    BackendConfig* find_backend(const std::string& path);
    void forward_request(const http::request<http::string_body>& req, 
                        BackendConfig& backend,
                        http::response<http::string_body>& res);
};
