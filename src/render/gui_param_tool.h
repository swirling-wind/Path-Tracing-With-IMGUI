#pragma once

#include <string>
#include <vector>

#include <filesystem>
#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>
#include <cstdio>

namespace gui_tools
{
    constexpr int num_of_material_list = 12;
    std::array<const char*, num_of_material_list> material_list = {
        "default", "light",  "red_wall", "green_wall", "blue_wall",
        "water", "glass", "floor", "wood", "iron",
        "copper", "gold"
    };

    struct object_imported
    {
        std::array<float, 3> position{ 0.0,0.0,0.0 };
        std::filesystem::path file_location; // path
        int material_type{ 0 };
    };

    nlohmann::json generate_render_params(const std::array<float, 3>camera_eye_pos, const std::array<float, 3>camera_look_at,
        std::array<int, 2> output_image_size, const int samples_per_pixel,
        std::vector<object_imported> objects_vector)
    {
        nlohmann::json render_params{
        { "num_render_threads", -1},
        {  "ior" , 1},
            {"cameras",
                {
                    {"focal_length", 50},
                    {"sensor_width" , 35},
                    {"eye", {camera_eye_pos.at(0), camera_eye_pos.at(1), camera_eye_pos.at(2)} },
                    {"look_at", {camera_look_at.at(0), camera_look_at.at(1), camera_look_at.at(2)} },
                    {"image",
                        {
                            {"width", output_image_size.at(0)},
                            {"height", output_image_size.at(1)},
                            { "exposure_compensation", -1.5,},
                            {"gain_compensation", 0.0},
                            {"tonemapper", "ACES"}
                            }
                        },
                    {"sqrtspp", samples_per_pixel}
                    }
                },
            {"bvh",
                {"type", "octree"}
            }

        };
        return render_params;
    }
}


