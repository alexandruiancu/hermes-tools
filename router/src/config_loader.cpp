#include "config_loader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

// Static member initialization
std::vector<EndpointConfig> ConfigLoader::endpoints_;
std::map<std::string, std::vector<std::string>> ConfigLoader::api_routes_;
ServerConfig ConfigLoader::server_config_;
bool ConfigLoader::loaded_ = false;

// Simple JSON value extraction helpers
static std::string extract_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    pos = json.find('"', pos + search.length());
    if (pos == std::string::npos) return "";
    pos++;
    
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return "";
    
    return json.substr(pos, end - pos);
}

static int extract_int(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return 0;
    
    pos = json.find(':', pos + search.length());
    if (pos == std::string::npos) return 0;
    pos++;
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    // Extract number
    size_t end = pos;
    while (end < json.length() && (std::isdigit(json[end]) || json[end] == '-')) end++;
    
    try {
        return std::stoi(json.substr(pos, end - pos));
    } catch (...) {
        return 0;
    }
}

static bool extract_bool(const std::string& json, const std::string& key, bool default_val = false) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return default_val;
    
    pos = json.find(':', pos + search.length());
    if (pos == std::string::npos) return default_val;
    pos++;
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    if (pos < json.length() && json[pos] == 't') return true;
    if (pos < json.length() && json[pos] == 'f') return false;
    
    return default_val;
}

bool ConfigLoader::load(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << config_path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    
    parse_endpoints(json);
    parse_api_routes(json);
    parse_server_config(json);
    
    loaded_ = true;
    std::cout << "Configuration loaded from: " << config_path << std::endl;
    return true;
}

void ConfigLoader::parse_endpoints(const std::string& json) {
    // Find "endpoints" section
    size_t start = json.find("\"endpoints\"");
    if (start == std::string::npos) return;
    
    size_t brace_start = json.find('{', start);
    if (brace_start == std::string::npos) return;
    
    // Find matching closing brace (simplified)
    int depth = 0;
    size_t brace_end = brace_start;
    for (size_t i = brace_start; i < json.length(); i++) {
        if (json[i] == '{') depth++;
        if (json[i] == '}') depth--;
        if (depth == 0) {
            brace_end = i + 1;
            break;
        }
    }
    
    std::string endpoints_section = json.substr(brace_start, brace_end - brace_start);
    
    // Parse each endpoint
    size_t pos = 0;
    while ((pos = endpoints_section.find("\"", pos)) != std::string::npos) {
        size_t name_end = endpoints_section.find("\"", pos + 1);
        if (name_end == std::string::npos) break;
        
        std::string name = endpoints_section.substr(pos + 1, name_end - pos - 1);
        if (name == "endpoints" || name == "api_routes" || name == "server") {
            pos = name_end + 1;
            continue;
        }
        
        // Find the endpoint object
        size_t obj_start = endpoints_section.find('{', name_end);
        if (obj_start == std::string::npos) break;
        
        int depth = 0;
        size_t obj_end = obj_start;
        for (size_t i = obj_start; i < endpoints_section.length(); i++) {
            if (endpoints_section[i] == '{') depth++;
            if (endpoints_section[i] == '}') depth--;
            if (depth == 0) {
                obj_end = i + 1;
                break;
            }
        }
        
        std::string obj = endpoints_section.substr(obj_start, obj_end - obj_start);
        
        EndpointConfig ep;
        ep.name = name;
        ep.host = extract_string(obj, "host");
        ep.port = static_cast<unsigned short>(extract_int(obj, "port"));
        ep.path_prefix = extract_string(obj, "path_prefix");
        ep.enabled = extract_bool(obj, "enabled", true);
        ep.ssl = extract_bool(obj, "ssl", false);
        
        endpoints_.push_back(ep);
        
        pos = obj_end;
    }
}

void ConfigLoader::parse_api_routes(const std::string& json) {
    size_t start = json.find("\"api_routes\"");
    if (start == std::string::npos) return;
    
    size_t brace_start = json.find('{', start);
    if (brace_start == std::string::npos) return;
    
    int depth = 0;
    size_t brace_end = brace_start;
    for (size_t i = brace_start; i < json.length(); i++) {
        if (json[i] == '{') depth++;
        if (json[i] == '}') depth--;
        if (depth == 0) {
            brace_end = i + 1;
            break;
        }
    }
    
    std::string routes_section = json.substr(brace_start, brace_end - brace_start);
    
    // Parse each service routes
    size_t pos = 0;
    while ((pos = routes_section.find("\"", pos)) != std::string::npos) {
        size_t name_end = routes_section.find("\"", pos + 1);
        if (name_end == std::string::npos) break;
        
        std::string name = routes_section.substr(pos + 1, name_end - pos - 1);
        if (name == "api_routes") {
            pos = name_end + 1;
            continue;
        }
        
        // Find the routes array
        size_t arr_start = routes_section.find('[', name_end);
        if (arr_start == std::string::npos) break;
        
        size_t arr_end = routes_section.find(']', arr_start);
        if (arr_end == std::string::npos) break;
        
        std::string arr = routes_section.substr(arr_start + 1, arr_end - arr_start - 1);
        
        // Extract routes from array
        size_t arr_pos = 0;
        while ((arr_pos = arr.find("\"", arr_pos)) != std::string::npos) {
            size_t route_end = arr.find("\"", arr_pos + 1);
            if (route_end == std::string::npos) break;
            
            std::string route = arr.substr(arr_pos + 1, route_end - arr_pos - 1);
            api_routes_[name].push_back(route);
            
            arr_pos = route_end + 1;
        }
        
        pos = arr_end + 1;
    }
}

void ConfigLoader::parse_server_config(const std::string& json) {
    size_t start = json.find("\"server\"");
    if (start == std::string::npos) return;
    
    size_t brace_start = json.find('{', start);
    if (brace_start == std::string::npos) return;
    
    int depth = 0;
    size_t brace_end = brace_start;
    for (size_t i = brace_start; i < json.length(); i++) {
        if (json[i] == '{') depth++;
        if (json[i] == '}') depth--;
        if (depth == 0) {
            brace_end = i + 1;
            break;
        }
    }
    
    std::string server_obj = json.substr(brace_start, brace_end - brace_start);
    
    server_config_.host = extract_string(server_obj, "host");
    server_config_.port = static_cast<unsigned short>(extract_int(server_obj, "port"));
    
    if (server_config_.host.empty()) server_config_.host = "0.0.0.0";
    if (server_config_.port == 0) server_config_.port = 8080;
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
