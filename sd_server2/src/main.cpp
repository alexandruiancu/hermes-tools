#include <crow.h>
#include <stb_image_write.h>
#include <vector>
#include <thread>
#include <memory>
int main()
{
    // 1) Load the model once
    std::unique_ptr<sd::Context> ctx = std::make_unique<sd::Context>();
    ctx->load("models/flux/flux1.safetensors");  // path to a downloaded model
    // 2) Start Crow
    crow::SimpleApp app;
    CROW_ROUTE(app, "/sd").methods("POST"_method)([ctx = std::move(ctx)](const crow::request& req, crow::response& res){
        try {
            // parse JSON
            auto json = crow::json::load(req.body);
            std::string prompt = json["prompt"].s();
            int steps = json.has("steps") ? json["steps"].i() : 30;
            float cfg   = json.has("cfg_scale") ? json["cfg_scale"].f() : 7.5f;
            int w = json.has("width")  ? json["width"].i() : 512;
            int h = json.has("height") ? json["height"].i() : 512;
            // run inference
            std::vector<uint8_t> raw = ctx->infer(prompt, steps, cfg, w, h);
            // encode PNG
            std::vector<uint8_t> png;
            int stride = 3 * w;
            stbi_write_png_to_func(
                [](void* context, void* data, int size) {
                    std::vector<uint8_t>* buf = (std::vector<uint8_t>*)context;
                    buf->insert(buf->end(), (uint8_t*)data, (uint8_t*)data + size);
                },
                &png,
                w, h, 3, raw.data(), stride);
            // base64‑encode
            std::string png_b64 = crow::b64::encode(png.data(), png.size());
            // reply
            crow::json::wvalue out;
            out["image"] = png_b64;
            res.write(out);
            res.code = 200;
        } catch (const std::exception& e) {
            res.code = 500;
            res.write(std::string("Error: ") + e.what());
        }
    });
    app.port(8080).multithreaded().run();
}