#include "metrics.hpp"
#include <iostream>

std::mutex Metrics::mutex_;
std::map<std::string, int> Metrics::request_counts_;
std::map<std::string, std::vector<double>> Metrics::latencies_;
std::map<std::string, std::pair<int, int>> Metrics::token_counts_;
std::map<std::string, ModelMetrics> Metrics::model_metrics_;

void Metrics::record_request(const std::string& service, const std::string& method) {
    std::lock_guard<std::mutex> lock(mutex_);
    request_counts_[service]++;
    
    std::cout << "[METRICS] Request to " << service << " (" << method << ")" << std::endl;
}

void Metrics::record_latency(const std::string& service, double ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    latencies_[service].push_back(ms);
    
    std::cout << "[METRICS] Latency for " << service << ": " << ms << "ms" << std::endl;
}

void Metrics::record_tokens(const std::string& service, int input_tokens, int output_tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    token_counts_[service] = {input_tokens, output_tokens};
    
    std::cout << "[METRICS] Tokens for " << service << ": in=" << input_tokens 
              << ", out=" << output_tokens << std::endl;
}

void Metrics::record_model_loaded(const std::string& model_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    model_metrics_[model_name].loaded = true;
    std::cout << "[METRICS] Model loaded: " << model_name << std::endl;
}

void Metrics::record_model_unloaded(const std::string& model_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (model_metrics_.count(model_name)) {
        model_metrics_[model_name].loaded = false;
    }
    std::cout << "[METRICS] Model unloaded: " << model_name << std::endl;
}

std::map<std::string, int> Metrics::get_request_counts() {
    std::lock_guard<std::mutex> lock(mutex_);
    return request_counts_;
}

std::map<std::string, double> Metrics::get_avg_latency() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::map<std::string, double> avg;
    
    for (const auto& [service, latencies] : latencies_) {
        if (!latencies.empty()) {
            double sum = 0;
            for (double l : latencies) sum += l;
            avg[service] = sum / latencies.size();
        }
    }
    
    return avg;
}

std::map<std::string, ModelMetrics> Metrics::get_model_metrics() {
    std::lock_guard<std::mutex> lock(mutex_);
    return model_metrics_;
}

ModelMetrics Metrics::get_model_metrics(const std::string& model_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (model_metrics_.count(model_name)) {
        return model_metrics_[model_name];
    }
    return ModelMetrics{};
}

std::vector<std::string> Metrics::get_all_model_names() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const auto& [name, _] : model_metrics_) {
        names.push_back(name);
    }
    return names;
}
