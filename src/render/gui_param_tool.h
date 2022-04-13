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
    constexpr int num_of_material_list = 11;
    inline std::array<const char*, num_of_material_list> material_list = {
        "default", "light",  "wood", "walnut", "oak",
        "gold", "nickel", "copper", "iron", "brass",
        "palladium"
    };

    struct object_imported
    {
        std::array<float, 3> position{ 0.0,0.0,0.0 };
        std::filesystem::path file_location; // path
        int material_type{ 0 };
    };


    inline std::vector<std::filesystem::path> get_models_in_folder(const std::filesystem::path folder_path)
    {
        std::vector<std::filesystem::path> obj_path_vector;
        for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(folder_path))
        {
            if (!file.path().has_extension() || file.path().extension() != ".obj")
                continue;
            obj_path_vector.push_back(file.path());
            std::string obj_file_name = file.path().filename().string();
            std::cout << obj_file_name << std::endl;

        }
        return obj_path_vector;
    }

    inline nlohmann::json generate_render_params(std::string save_name,
                                                 const std::array<float, 3>camera_eye_pos, const std::array<float, 3>camera_look_at,
                                                 std::array<int, 2> output_image_size, const int samples_per_pixel,
                                                 std::vector<object_imported> objects_vector)
    {



        nlohmann::json render_params;
        render_params["num_render_threads"] = -1;
        render_params["ior"] = 1;

        //camera
        nlohmann::json camera_params;
        camera_params["focal_length"] = 50;
        camera_params["sensor_width"] = 35;  
        camera_params["eye"] = camera_eye_pos;
        camera_params["look_at"] = camera_look_at;

            //Image
            nlohmann::json image_params;
            image_params["width"] = output_image_size.at(0);
            image_params["height"] = output_image_size.at(1);
            image_params["exposure_compensation"] = -1.5;
            image_params["gain_compensation"] = 0.0;
            image_params["tonemapper"] = "ACES";
            camera_params["image"] = image_params;

        camera_params["sqrtspp"] = samples_per_pixel;
        camera_params["savename"] = save_name;

        render_params["camera"] = camera_params;


        //BVH
        nlohmann::json bvh_type;
        bvh_type["type"] = "octree";
        render_params["bvh"] = bvh_type;

        //Materials
        nlohmann::json material_params;

        nlohmann::json default_params;
        default_params["reflectance"] = 0.95;
        material_params["default"] = default_params;

        nlohmann::json light_params;
        light_params["emittance"] = { 235.1913,195.1469,144.3919 };
        material_params["light"] = light_params;

        nlohmann::json wood_params;
        wood_params["reflectance"] = "#DECBB1";
        material_params["wood"] = wood_params;

        nlohmann::json walnut_params;
        walnut_params["reflectance"] = "#804000";
        material_params["walnut"] = walnut_params;

        nlohmann::json oak_params;
        oak_params["reflectance"] = "#E0AC69";
        material_params["oak"] = oak_params;

        nlohmann::json gold_params;
        gold_params["specular_roughness"] = 0.15;
        nlohmann::json gold_ior;
        gold_ior["real"] = { 0.03344755, 0.36314684, 1.61295201};
        gold_ior["imaginary"] = { 3.90181846, 2.43300728, 1.79303367 };
        gold_params["ior"] = gold_ior;
        material_params["gold"] = gold_params;

        nlohmann::json nickel_params;
        nickel_params["specular_roughness"] = 0.17;
        nlohmann::json nickle_ior;
        nickle_ior["real"] = { 2.02840832, 1.90465356, 1.72136251 };
        nickle_ior["imaginary"] = { 4.20416428, 3.57426042, 2.87298213 };
        nickel_params["ior"] = nickle_ior;
        material_params["nickel"] = nickel_params;

        nlohmann::json copper_params;
        copper_params["specular_roughness"] = 0.25;
        nlohmann::json copper_ior;
        copper_ior["real"] = { -0.16968940, 0.79400849, 1.10737582 };
        copper_ior["imaginary"] = { 3.53627871, 2.59968455, 2.37851486 };
        copper_params["ior"] = copper_ior;
        material_params["copper"] = copper_params;

        nlohmann::json iron_params;
        iron_params["specular_roughness"] = 0.19;
        nlohmann::json iron_ior;
        iron_ior["real"] = { 2.91679227, 2.92517616, 2.53774810 };
        iron_ior["imaginary"] = { 3.08474983, 2.93861411, 2.74620057 };
        iron_params["ior"] = iron_ior;
        material_params["iron"] = iron_params;

        nlohmann::json brass_params;
        brass_params["specular_roughness"] = 0.21;
        nlohmann::json brass_ior;
        brass_ior["real"] = { 0.18876745, 0.57522280, 1.03318002 };
        brass_ior["imaginary"] = { 3.38620319, 2.38383535, 1.87526994 };
        brass_params["ior"] = brass_ior;
        material_params["brass"] = brass_params;

        nlohmann::json palladium_params;
        palladium_params["specular_roughness"] = 0.23;
        nlohmann::json palladium_ior;
        palladium_ior["real"] = { 1.79598558, 1.62313143, 1.38800161 };
        palladium_ior["imaginary"] = { 4.37307307, 3.81575961, 3.19347675 };
        palladium_params["ior"] = palladium_ior;
        material_params["palladium"] = palladium_params;

        render_params["materials"] = material_params;

        //surfaces
        nlohmann::json surfaces_params(nlohmann::json::value_t::array);

        nlohmann::json default_light;
        default_light["type"] = "sphere";
        default_light["material"] = "light";
        default_light["radius"] = 0.5;
        default_light["position"] = { 1.0, 5.1, 4.2 };
        surfaces_params.insert(surfaces_params.end(), default_light);

        for (const auto& object : objects_vector)
        {
            nlohmann::json obj_json;

            obj_json["type"] = "object";
            obj_json["material"] = material_list.at(object.material_type);
            obj_json["smooth"] = true;
            obj_json["position"] = object.position;
            obj_json["file"] = object.file_location.string();

            surfaces_params.insert(surfaces_params.end(), obj_json);
        }

        render_params["surfaces"] = surfaces_params;

        return render_params;
    }
}


