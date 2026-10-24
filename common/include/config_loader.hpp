#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

struct EndpointConfig {
    std::string name;
    std::string host;
    unsigned short port;
    bool enabled = true;
    bool ssl = false;
    std::string api_key;
    std::string sd_model_path;  // Added: for Stable Diffusion model path
};

struct ServerConfig {
    std::string host;
    unsigned short port;
};

class ConfigLoader {
public:
    // Load configuration from file
    static bool load(const std::string& config_path);
    
    // Get loaded configuration
    static std::vector<EndpointConfig> get_endpoints();
    static std::map<std::string, std::vector<std::string>> get_api_routes();
    static ServerConfig get_server_config();
    
    // Check if configuration is loaded
    static bool is_loaded();
    
private:
    static std::vector<EndpointConfig> endpoints_;
    static std::map<std::string, std::vector<std::string>> api_routes_;
    static ServerConfig server_config_;
    static bool loaded_;
    
    // Internal parsing functions
    static void parse_endpoints(const nlohmann::json& json);
    static void parse_api_routes(const nlohmann::json& json);
    static void parse_server_config(const nlohmann::json& json);
};
