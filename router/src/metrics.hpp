#pragma once
#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>

class Metrics {
public:
    static void record_request(const std::string& service, const std::string& method);
    static void record_latency(const std::string& service, double ms);
    static void record_tokens(const std::string& service, int input_tokens, int output_tokens);
    
    static std::map<std::string, int> get_request_counts();
    static std::map<std::string, double> get_avg_latency();
    
private:
    static std::mutex mutex_;
    static std::map<std::string, int> request_counts_;
    static std::map<std::string, std::vector<double>> latencies_;
    static std::map<std::string, std::pair<int, int>> token_counts_;
};
