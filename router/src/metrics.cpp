#include "metrics.hpp"
#include <iostream>

std::mutex Metrics::mutex_;
std::map<std::string, int> Metrics::request_counts_;
std::map<std::string, std::vector<double>> Metrics::latencies_;
std::map<std::string, std::pair<int, int>> Metrics::token_counts_;

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
