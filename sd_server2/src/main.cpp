#include <crow.h>
// Common helpers shared with other sd_server* projects
#include "server_utils.hpp"

int main()
{
    // 1) Load the model once
    // NOTE: The sd inference library is not part of this repository.
    // Replace the following with the actual implementation when available.
    struct DummyCtx {
        void load(const std::string&) {}
        std::vector<uint8_t> infer(const std::string&, int, float, int, int) { return {}; }
    } ctx;
    // 2) Start Crow
    crow::SimpleApp app;
    CROW_ROUTE(app, "/sd").methods("POST"_method)([&](const crow::request& req, crow::response& res){
        try {
            auto json = crow::json::load(req.body);
            ImageRequest r = ImageRequest::from_json(json);

            // run inference
            std::vector<uint8_t> raw = ctx.infer(r.prompt, r.steps, r.cfg_scale, r.width, r.height);
            write_image_response(res, raw);
        } catch (const std::exception& e) {
            set_error(res, 500, e.what());
        }
    });
    app.port(8080).multithreaded().run();
}