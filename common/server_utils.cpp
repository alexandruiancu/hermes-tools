#include "server_utils.hpp"
#include <crow/utility.h>
//#include <stb_image_write.h>

ImageRequest ImageRequest::from_json(crow::json::rvalue json) {
    ImageRequest req;
    if (!json.has("prompt")) throw std::runtime_error("Missing 'prompt'");
    req.prompt = json["prompt"].s();

    // Optional overrides – keep the defaults if not present
    if (json.has("steps"))   req.steps   = json["steps"].i();
    if (json.has("cfg_scale")) req.cfg_scale = json["cfg_scale"].d();
    if (json.has("width"))   req.width   = json["width"].i();
    if (json.has("height"))  req.height  = json["height"].i();

    return req;
}

void write_image_response(crow::response& res, const std::vector<uint8_t>& raw_png) {
    // Encode PNG to base64
    std::string b64 = crow::utility::base64encode(raw_png.data(), raw_png.size());

    crow::json::wvalue out;
    out["image"] = b64;          // key name can be changed per project
    res.write(out.dump());
}
