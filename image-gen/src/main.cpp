#include <crow.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <array>
#include <cstdio>
#include <iostream>

// Run the llama-diffusion-cli binary and capture its stdout (base64 PNG)
std::string run_cli(const std::string &prompt, const std::string &extra = "") {
    std::string cmd = "/app/llama-diffusion-cli -m dream7b.gguf -p \"" + prompt + "\" -ub 512";
    if (!extra.empty()) cmd += " " + extra;
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("failed to run CLI");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result; // expected to be a base64 PNG string
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/generate").methods("POST")( [](const crow::request &req) {
        // Parse JSON body
        crow::json::wvalue body = crow::json::load(req.body);
        std::string prompt = body["prompt"].s();
        if (prompt.empty()) return crow::response(400, "Missing 'prompt'");

        // Optional overrides via query params
        std::string extra;
        if (body.has("steps"))   extra += " --diffusion-steps " + body["steps"].s();
        if (body.has("eps"))     extra += " --diffusion-eps " + body["eps"].s();
        if (body.has("algorithm")) extra += " --diffusion-algorithm " + body["algorithm"].s();

        try {
            std::string image_b64 = run_cli(prompt, extra);
            crow::json::wvalue resp;
            resp["image_base64"] = image_b64;
            return resp;
        } catch (const std::exception &e) {
            return crow::response(500, e.what());
        }
    });

    app.port(8000).multithreaded().run();
    return 0;
}
