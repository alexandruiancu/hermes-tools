#include <crow.h>
#include <stable-diffusion.h>
// Common helpers shared with other sd_server* projects
#include "server_utils.hpp"
#include <memory>
#include <mutex>

struct SDContext {
    sd_ctx_t* ctx = nullptr;
    std::mutex mtx;
    
    ~SDContext() {
        if (ctx) {
            free_sd_ctx(ctx);
        }
    }
};

int main()
{
    // 1) Initialize stable-diffusion context
    sd_ctx_params_t params;
    sd_ctx_params_init(&params);
    
    // Set model path - you'll need to provide the actual model path
    // For now, we'll use a default that should be overridden via config
    params.model_path = std::getenv("SD_MODEL_PATH") ? std::getenv("SD_MODEL_PATH") : "/models/sd-model.gguf";
    params.n_threads = std::thread::hardware_concurrency();
    params.wtype = SD_TYPE_Q4_K;  // Default quantization
    params.rng_type = STD_DEFAULT_RNG;
    params.enable_mmap = true;
    params.flash_attn = true;
    
    SDContext sd_ctx;
    sd_ctx.ctx = new_sd_ctx(&params);
    
    if (!sd_ctx.ctx) {
        std::cerr << "Failed to create stable-diffusion context. Model path: " << params.model_path << std::endl;
        return 1;
    }
    
    std::cout << "Model loaded: " << sd_get_model_version_name(sd_ctx.ctx) << std::endl;
    
    // 2) Start Crow
    crow::SimpleApp app;
    CROW_ROUTE(app, "/sd").methods("POST"_method)([&](const crow::request& req, crow::response& res){
        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                set_error(res, 400, "Invalid JSON");
                return;
            }
            
            ImageRequest r = ImageRequest::from_json(json);

            // Prepare generation parameters
            sd_img_gen_params_t gen_params;
            sd_img_gen_params_init(&gen_params);
            
            gen_params.prompt = r.prompt.c_str();
            gen_params.negative_prompt = "";
            gen_params.width = r.width;
            gen_params.height = r.height;
            gen_params.sample_params.sample_steps = r.steps;
            gen_params.sample_params.guidance.txt_cfg = r.cfg_scale;
            gen_params.seed = -1;  // Random seed
            gen_params.batch_count = 1;
            gen_params.strength = 1.0f;
            gen_params.clip_skip = 1;
            
            // Set default sampler
            gen_params.sample_params.sample_method = sd_get_default_sample_method(sd_ctx.ctx);
            gen_params.sample_params.scheduler = sd_get_default_scheduler(sd_ctx.ctx, gen_params.sample_params.sample_method);
            
            // Run inference
            sd_image_t* images_out = nullptr;
            int num_images_out = 0;
            
            {
                std::lock_guard<std::mutex> lock(sd_ctx.mtx);
                bool success = generate_image(sd_ctx.ctx, &gen_params, &images_out, &num_images_out);
                
                if (!success || num_images_out == 0 || !images_out) {
                    set_error(res, 500, "Image generation failed");
                    return;
                }
            }
            
            // Convert the first generated image to PNG bytes
            // The image data is in RGBA format
            sd_image_t& img = images_out[0];
            std::vector<uint8_t> png_data(img.width * img.height * 4);
            std::memcpy(png_data.data(), img.data, img.width * img.height * 4);
            
            // Free the generated images
            // Note: stable-diffusion.cpp allocates the image data, we need to free it
            // The library manages the memory, so we don't free img.data directly
            
            write_image_response(res, png_data);
            
        } catch (const std::exception& e) {
            set_error(res, 500, e.what());
        }
    });
    
    // Health check endpoint
    CROW_ROUTE(app, "/health").methods("GET"_method)([](){
        return crow::response(200, "OK");
    });
    
    app.port(8080).multithreaded().run();
}