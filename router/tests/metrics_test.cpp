#include "metrics.hpp"
#include <cassert>
#include <iostream>

int main() {
    // Record values matching the llama-cpp output
    Metrics::record_request("llama-cpp", "chat");
    Metrics::record_latency("llama-cpp", 50.0);
    Metrics::record_tokens("llama-cpp", 1000, 500);
    Metrics::record_model_loaded("Nemotron-3.5-Lightning-30B-A3B");

    // New metrics from the output
    Metrics::record_prompt_tokens_cached("llama-cpp", 229461LL);
    Metrics::record_prompt_seconds("llama-cpp", 96.3788);
    Metrics::record_tokens_predicted("llama-cpp", 4475LL);
    Metrics::record_tokens_predicted_seconds("llama-cpp", 547.506);
    Metrics::record_n_decode("llama-cpp");
    Metrics::record_n_tokens_max("llama-cpp", 47244LL);
    Metrics::record_spec_decode_draft_tokens("llama-cpp", 0LL);
    Metrics::record_spec_decode_accepted_tokens("llama-cpp", 0LL);
    Metrics::record_spec_decode_drafts("llama-cpp", 0LL);
    Metrics::record_requests_processing("llama-cpp", 3);
    Metrics::record_requests_deferred("llama-cpp", 0);
    Metrics::record_n_busy_slots_per_decode("llama-cpp", 1.46293);

    // Verify getters
    assert(Metrics::get_request_counts().count("llama-cpp") > 0);
    std::cout << "request count for llama-cpp: "
              << Metrics::get_request_counts()["llama-cpp"] << std::endl;

    assert(Metrics::get_prompt_tokens_cached_total() == 229461LL);
    std::cout << "prompt_tokens_cached_total: "
              << Metrics::get_prompt_tokens_cached_total() << std::endl;

    assert(Metrics::get_prompt_seconds_total() == static_cast<long long>(96.3788));
    std::cout << "prompt_seconds_total: "
              << Metrics::get_prompt_seconds_total() << std::endl;

    assert(Metrics::get_tokens_predicted_total() == 4475LL);
    std::cout << "tokens_predicted_total: "
              << Metrics::get_tokens_predicted_total() << std::endl;

    assert(Metrics::get_tokens_predicted_seconds_total() == static_cast<long long>(547.506));
    std::cout << "tokens_predicted_seconds_total: "
              << Metrics::get_tokens_predicted_seconds_total() << std::endl;

    assert(Metrics::get_n_decode_total() == 1LL); // we called once
    std::cout << "n_decode_total: " << Metrics::get_n_decode_total() << std::endl;

    assert(Metrics::get_n_tokens_max() == 47244LL);
    std::cout << "n_tokens_max: " << Metrics::get_n_tokens_max() << std::endl;

    assert(Metrics::get_spec_decode_draft_tokens_total() == 0LL);
    std::cout << "spec_decode_draft_tokens_total: "
              << Metrics::get_spec_decode_draft_tokens_total() << std::endl;

    assert(Metrics::get_spec_decode_accepted_tokens_total() == 0LL);
    std::cout << "spec_decode_accepted_tokens_total: "
              << Metrics::get_spec_decode_accepted_tokens_total() << std::endl;

    assert(Metrics::get_spec_decode_drafts_total() == 0LL);
    std::cout << "spec_decode_drafts_total: "
              << Metrics::get_spec_decode_drafts_total() << std::endl;

    assert(Metrics::get_requests_processing() == 3);
    std::cout << "requests_processing: " << Metrics::get_requests_processing() << std::endl;

    assert(Metrics::get_requests_deferred() == 0);
    std::cout << "requests_deferred: " << Metrics::get_requests_deferred() << std::endl;

    assert(Metrics::get_n_busy_slots_per_decode() == 1.46293);
    std::cout << "n_busy_slots_per_decode: "
              << Metrics::get_n_busy_slots_per_decode() << std::endl;

    std::cout << "All metrics unit tests passed!" << std::endl;
    return 0;
}
