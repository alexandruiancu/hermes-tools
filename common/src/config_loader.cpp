#include "config_loader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>

// Static member initialization
std::vector<EndpointConfig> ConfigLoader::endpoints_;
std::map<std::string, std::vector<std::string>> ConfigLoader::api_routes_;
ServerConfig ConfigLoader::server_config_;
bool ConfigLoader::loaded_ = false;

bool ConfigLoader::load(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << config_path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    try {
        nlohmann::json json = nlohmann::json::parse(content);
        
        parse_endpoints(json);
        parse_api_routes(json);
        parse_server_config(json);
        
        loaded_ = true;
        std::cout << "Configuration loaded from: " << config_path << std::endl;
        return true;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error: Failed to parse config JSON: " << e.what() << std::endl;
        return false;
    }
}

void ConfigLoader::parse_endpoints(const nlohmann::json& json) {
    if (!json.contains("endpoints") || !json["endpoints"].is_object()) {
        std::cerr << "Config missing 'endpoints' object" << std::endl;
        return;
    }
    
    const auto& endpoints = json["endpoints"];
    for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
        EndpointConfig ep;
        ep.name = it.key();
        const auto& obj = it.value();
        
        ep.host = obj.value("host", "");
        ep.port = static_cast<unsigned short>(obj.value("port", 0));
        ep.enabled = obj.value("enabled", true);
        ep.ssl = obj.value("ssl", false);
        ep.api_key = obj.value("api_key", "");
        ep.sd_model_path = obj.value("sd_model_path", "");  // Added: parse model path
        
        endpoints_.push_back(ep);
    }
}

void ConfigLoader::parse_api_routes(const nlohmann::json& json) {
    if (!json.contains("api_routes") || !json["api_routes"].is_object()) {
        std::cerr << "Config missing 'api_routes' object" << std::endl;
        return;
    }
    
    const auto& routes = json["api_routes"];
    for (auto it = routes.begin(); it != routes.end(); ++it) {
        std::vector<std::string> route_list;
        const auto& arr = it.value();
        if (arr.is_array()) {
            for (const auto& item : arr) {
                if (item.is_string()) {
                    route_list.push_back(item.get<std::string>());
                }
            }
        }
        api_routes_[it.key()] = route_list;
    }
}

void ConfigLoader::parse_server_config(const nlohmann::json& json) {
    if (!json.contains("server") || !json["server"].is_object()) {
        std::cerr << "Config missing 'server' object" << std::endl;
        return;
    }
    
    const auto& server = json["server"];
    server_config_.host = server.value("host", "0.0.0.0");
    server_config_.port = static_cast<unsigned short>(server.value("port", 12345));
    
    if (server_config_.host.empty()) server_config_.host = "0.0.0.0";
    if (server_config_.port == 0) server_config_.port = 12345;
}

std::vector<EndpointConfig> ConfigLoader::get_endpoints() {
    return endpoints_;
}

std::map<std::string, std::vector<std::string>> ConfigLoader::get_api_routes() {
    return api_routes_;
}

ServerConfig ConfigLoader::get_server_config() {
    return server_config_;
}

bool ConfigLoader::is_loaded() {
    return loaded_;
}
