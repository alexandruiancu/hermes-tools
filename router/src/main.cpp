#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "proxy.hpp"
#include "metrics.hpp"
#include "config_loader.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void handle_session(tcp::socket socket) {
    try {
        beast::error_code ec;
        beast::flat_buffer buffer;

        for (;;) {
            http::request<http::string_body> req;
            http::read(socket, buffer, req, ec);
            if (ec == net::error::eof) break;
            if (ec) {
                std::cerr << "Read error: " << ec.message() << std::endl;
                break;
            }

            http::response<http::string_body> res;
            ProxyHandler handler;
            handler.handle_request(req, res);

            res.prepare_payload();
            http::write(socket, res, ec);
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Session error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Load configuration from external file
    std::string config_path = "config.json";
    if (argc > 1) {
        config_path = argv[1];
    }
    
    if (!ConfigLoader::load(config_path)) {
        std::cerr << "Failed to load configuration from: " << config_path << std::endl;
        return 1;
    }
    
    // Get server configuration
    ServerConfig server_config = ConfigLoader::get_server_config();
    const unsigned short port = server_config.port;
    
    net::io_context ioc{1};
    
    // Create SSL context
    net::ssl::context ctx{net::ssl::context::tls_server};
    // ctx.set_password_callback(...);
    // ctx.use_certificate_file(...);
    // ctx.use_private_key_file(...);
    
    // Create acceptor
    tcp::acceptor acceptor{ioc, tcp::endpoint{tcp::v4(), port}};
    
    std::cout << "AI Router starting on " << server_config.host << ":" << port << std::endl;
    std::cout << "Loaded " << ConfigLoader::get_endpoints().size() << " endpoints" << std::endl;
    
    // Start accepting connections
    while (true) {
        tcp::socket socket{ioc};
        acceptor.accept(socket);
        
        // Handle connection in a new thread
        std::thread([socket = std::move(socket)]() mutable {
            handle_session(std::move(socket));
        }).detach();
    }
    
    return 0;
}
