#pragma once
#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <memory>

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
    static Metrics& getMetrics(const std::string& endpoint);
    static void record_prompt_tokens_cached(const std::string& service, long long cached);
    static void record_prompt_seconds(const std::string& service, double seconds);
    static void record_tokens_predicted(const std::string& service, long long predicted);
    static void record_tokens_predicted_seconds(const std::string& service, double seconds);
    static void record_n_decode(const std::string& service);
    static void record_n_tokens_max(const std::string& service, long long max);
    static void record_spec_decode_draft_tokens(const std::string& service, long long drafts);
    static void record_spec_decode_accepted_tokens(const std::string& service, long long accepted);
    static void record_spec_decode_drafts(const std::string& service, long long drafts);
    static void record_requests_processing(const std::string& service, int count);
    static void record_requests_deferred(const std::string& service, int deferred);
    static void record_n_busy_slots_per_decode(const std::string& service, double busy);
    static long long get_prompt_tokens_cached_total();
    static long long get_prompt_seconds_total();
    static long long get_tokens_predicted_total();
    static long long get_tokens_predicted_seconds_total();
    static long long get_n_decode_total();
    static long long get_n_tokens_max();
    static long long get_spec_decode_draft_tokens_total();
    static long long get_spec_decode_accepted_tokens_total();
    static long long get_spec_decode_drafts_total();
    static double get_prompt_tokens_seconds();
    static double get_predicted_tokens_seconds();
    static int get_requests_processing();
    static int get_requests_deferred();
    static double get_n_busy_slots_per_decode();
    
private:
    static std::mutex mutex_;
    static std::map<std::string, int> request_counts_;
    static std::map<std::string, std::vector<double>> latencies_;
    static std::map<std::string, std::pair<int, int>> token_counts_;
    static std::map<std::string, ModelMetrics> model_metrics_;
    static std::atomic<long long> prompt_tokens_cached_total_;
    static std::atomic<long long> prompt_seconds_total_;
    static std::atomic<long long> tokens_predicted_total_;
    static std::atomic<long long> tokens_predicted_seconds_total_;
    static std::atomic<long long> n_decode_total_;
    static std::atomic<long long> n_tokens_max_;
    static std::atomic<long long> spec_decode_num_draft_tokens_total_;
    static std::atomic<long long> spec_decode_num_accepted_tokens_total_;
    static std::atomic<long long> spec_decode_num_drafts_total_;
    static std::atomic<double> prompt_tokens_seconds_;
    static std::atomic<double> predicted_tokens_seconds_;
    static std::atomic<int> requests_processing_;
    static std::atomic<int> requests_deferred_;
    static std::atomic<double> n_busy_slots_per_decode_;
    static std::unordered_map<std::string, std::unique_ptr<Metrics>> endpoint_metrics_;
};
