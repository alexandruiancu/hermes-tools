#pragma once
#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>

struct ModelMetrics {
    int request_count = 0;
    std::vector<double> latencies;
    std::pair<int, int> token_counts = {0, 0};
    bool loaded = false;
};

class Metrics {
public:
    static void record_request(const std::string& service, const std::string& method);
    static void record_latency(const std::string& service, double ms);
    static void record_tokens(const std::string& service, int input_tokens, int output_tokens);
    static void record_model_loaded(const std::string& model_name);
    static void record_model_unloaded(const std::string& model_name);
    
    static std::map<std::string, int> get_request_counts();
    static std::map<std::string, double> get_avg_latency();
    static std::map<std::string, ModelMetrics> get_model_metrics();
    static ModelMetrics get_model_metrics(const std::string& model_name);
    static std::vector<std::string> get_all_model_names();
    
private:
    static std::mutex mutex_;
    static std::map<std::string, int> request_counts_;
    static std::map<std::string, std::vector<double>> latencies_;
    static std::map<std::string, std::pair<int, int>> token_counts_;
    static std::map<std::string, ModelMetrics> model_metrics_;
};
