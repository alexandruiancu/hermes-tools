#pragma once

#include <crow.h>
#include <string>
#include <vector>

/* ------------------------------------------------------------------
 *  Request helpers
 * ------------------------------------------------------------------ */
struct ImageRequest {
    std::string prompt;
    int steps{30};
    float cfg_scale{7.5f};
    int width{512};
    int height{512};

    // Parse a Crow JSON body into this struct.
    // Parse a Crow JSON to this struct.
    static ImageRequest from_json(crow::json::rvalue json);
};

/* ------------------------------------------------------------------
 *  Response helpers
 * ------------------------------------------------------------------ */
void write_image_response(crow::response& res, const std::vector<uint8_t>& raw_png);

/* ------------------------------------------------------------------
 *  Error handling
 * ------------------------------------------------------------------ */
inline void set_error(crow::response& res, int code, const std::string& msg) {
    res.code = code;
    res.write(msg);
}
