#include "metrics.hpp"
#include <iostream>
#include <algorithm>

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

// Definitions for new metrics counters
std::atomic<long long> Metrics::prompt_tokens_cached_total_;
std::atomic<long long> Metrics::prompt_seconds_total_;
std::atomic<long long> Metrics::tokens_predicted_total_;
std::atomic<long long> Metrics::tokens_predicted_seconds_total_;
std::atomic<long long> Metrics::n_decode_total_;
std::atomic<long long> Metrics::n_tokens_max_;
std::atomic<long long> Metrics::spec_decode_num_draft_tokens_total_;
std::atomic<long long> Metrics::spec_decode_num_accepted_tokens_total_;
std::atomic<long long> Metrics::spec_decode_num_drafts_total_;
std::atomic<double> Metrics::prompt_tokens_seconds_;
std::atomic<double> Metrics::predicted_tokens_seconds_;
std::atomic<int> Metrics::requests_processing_;
std::atomic<int> Metrics::requests_deferred_;
std::atomic<double> Metrics::n_busy_slots_per_decode_;
std::unordered_map<std::string, std::unique_ptr<Metrics>> Metrics::endpoint_metrics_;
Metrics& Metrics::getMetrics(const std::string& endpoint) {
    if (endpoint_metrics_.find(endpoint) == endpoint_metrics_.end()) {
        endpoint_metrics_[endpoint] = std::make_unique<Metrics>();
    }
    return *endpoint_metrics_[endpoint];
}

// Record methods for new metrics
void Metrics::record_prompt_tokens_cached(const std::string& service, long long cached) {
    (void)service;
    prompt_tokens_cached_total_.fetch_add(cached);
}

void Metrics::record_prompt_seconds(const std::string& service, double seconds) {
    (void)service;
    prompt_seconds_total_.fetch_add(static_cast<long long>(seconds));
}

void Metrics::record_tokens_predicted(const std::string& service, long long predicted) {
    (void)service;
    tokens_predicted_total_.fetch_add(predicted);
}

void Metrics::record_tokens_predicted_seconds(const std::string& service, double seconds) {
    (void)service;
    tokens_predicted_seconds_total_.fetch_add(static_cast<long long>(seconds));
}

void Metrics::record_n_decode(const std::string& service) {
    (void)service;
    n_decode_total_.fetch_add(1);
}

void Metrics::record_n_tokens_max(const std::string& service, long long max) {
    (void)service;
    n_tokens_max_.store(std::max(n_tokens_max_.load(), max));
}

void Metrics::record_spec_decode_draft_tokens(const std::string& service, long long drafts) {
    (void)service;
    spec_decode_num_draft_tokens_total_.fetch_add(drafts);
}

void Metrics::record_spec_decode_accepted_tokens(const std::string& service, long long accepted) {
    (void)service;
    spec_decode_num_accepted_tokens_total_.fetch_add(accepted);
}

void Metrics::record_spec_decode_drafts(const std::string& service, long long drafts) {
    (void)service;
    spec_decode_num_drafts_total_.fetch_add(drafts);
}

void Metrics::record_requests_processing(const std::string& service, int count) {
    (void)service;
    requests_processing_.store(count);
}

void Metrics::record_requests_deferred(const std::string& service, int deferred) {
    (void)service;
    requests_deferred_.store(deferred);
}

void Metrics::record_n_busy_slots_per_decode(const std::string& service, double busy) {
    (void)service;
    n_busy_slots_per_decode_.store(busy);
}

// Get methods for new metrics
long long Metrics::get_prompt_tokens_cached_total() {
    return prompt_tokens_cached_total_.load();
}

long long Metrics::get_prompt_seconds_total() {
    return prompt_seconds_total_.load();
}

long long Metrics::get_tokens_predicted_total() {
    return tokens_predicted_total_.load();
}

long long Metrics::get_tokens_predicted_seconds_total() {
    return tokens_predicted_seconds_total_.load();
}

long long Metrics::get_n_decode_total() {
    return n_decode_total_.load();
}

long long Metrics::get_n_tokens_max() {
    return n_tokens_max_.load();
}

long long Metrics::get_spec_decode_draft_tokens_total() {
    return spec_decode_num_draft_tokens_total_.load();
}

long long Metrics::get_spec_decode_accepted_tokens_total() {
    return spec_decode_num_accepted_tokens_total_.load();
}

long long Metrics::get_spec_decode_drafts_total() {
    return spec_decode_num_drafts_total_.load();
}

double Metrics::get_prompt_tokens_seconds() {
    return prompt_tokens_seconds_.load();
}

double Metrics::get_predicted_tokens_seconds() {
    return predicted_tokens_seconds_.load();
}

int Metrics::get_requests_processing() {
    return requests_processing_.load();
}

int Metrics::get_requests_deferred() {
    return requests_deferred_.load();
}

double Metrics::get_n_busy_slots_per_decode() {
    return n_busy_slots_per_decode_.load();
}
