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
    constexpr int num_of_material_list = 18;

    inline std::array<const char*, num_of_material_list> material_list = {
        "default", "light",  "wood", "walnut", "oak",
        "gold", "nickel", "copper", "iron", "brass",
        "palladium", "glass",

        "shadow_green", "navy", "red", "green",

        "water", "F9_light"
    };

    struct shoot_params
    {
        std::array<float, 3>camera_eye_pos = { 0.0f, 1.5f, -8.0f };
        std::array<float, 3>camera_look_at = { 0.0f, 1.5f, 0.0f };

        std::array<int, 2> output_image_size = { 1280, 720 };
        std::array<int, 2> preview_image_size = { 1280, 720 };

        float focal_length = 50;
        float sensor_width = 35;
        float exposure_compensation = -1.5;
        float gain_compensation = 0.0;
        int side_spp = 3;
        bool is_photon_map = false;
        bool is_octree = true;
        bool is_sah = false;

        double photon_num = 1e6;
    };

    struct object_imported
    {
        std::array<float, 3> position{ 0.0,0.0,0.0 };
        std::filesystem::path file_location;
        int material_type{ 0 };
        std::array<float, 3> rotation{ 0.0,0.0,0.0 };
        std::array<float, 1> scale{ 1.0 };
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

    inline nlohmann::json generate_render_params(const shoot_params& shoot_image_params,
        std::string save_name,
        const std::array<float, 3>camera_eye_pos, const std::array<float, 3>camera_look_at,
        std::array<int, 2> output_image_size,
        std::vector<object_imported> objects_vector)
    {

        nlohmann::json render_params;
        render_params["num_render_threads"] = -1;
        render_params["ior"] = 1;

        //camera
        nlohmann::json camera_params(nlohmann::json::value_t::array);
        nlohmann::json one_instantce_camera;
        one_instantce_camera["focal_length"] = shoot_image_params.focal_length;
        one_instantce_camera["sensor_width"] = shoot_image_params.sensor_width;
        one_instantce_camera["eye"] = camera_eye_pos;
        one_instantce_camera["look_at"] = camera_look_at;

        //Image
        nlohmann::json image_params;
        image_params["width"] = output_image_size.at(0);
        image_params["height"] = output_image_size.at(1);
        image_params["exposure_compensation"] = shoot_image_params.exposure_compensation;
        image_params["gain_compensation"] = shoot_image_params.gain_compensation;
        image_params["tonemapper"] = "ACES";
        one_instantce_camera["image"] = image_params;

        one_instantce_camera["sqrtspp"] = shoot_image_params.side_spp;
        one_instantce_camera["savename"] = save_name;

        camera_params.insert(camera_params.end(), one_instantce_camera);
        render_params["cameras"] = camera_params;


        //BVH
        nlohmann::json bvh_type;
        if (shoot_image_params.is_octree)
        {
            bvh_type["type"] = "octree";
        }
        else
        {
            bvh_type["type"] = "quaternary_sah";
            bvh_type["bins_per_axis"] = 8;            
        }
        render_params["bvh"] = bvh_type;

        if (shoot_image_params.is_photon_map)
        {
            nlohmann::json photon_map_json;
            photon_map_json["emissions"] = floor(shoot_image_params.photon_num);
            photon_map_json["caustic_factor"] = 10.0;
            photon_map_json["k_nearest_photons"] = 50;
            photon_map_json["max_photons_per_octree_leaf"] = 200;
            photon_map_json["direct_visualization"] = false;
            render_params["photon_map"] = photon_map_json;
        }

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
        gold_ior["real"] = { 0.03344755, 0.36314684, 1.61295201 };
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

        nlohmann::json glass_params;
        glass_params["ior"] = 1.5;
        glass_params["transparancy"] = 1.0;
        material_params["glass"] = glass_params;

        //
        nlohmann::json shadow_green_params;
        shadow_green_params["reflectance"] = "#a2bf82";
        material_params["shadow_green"] = shadow_green_params;

        nlohmann::json navy_params;
        navy_params["reflectance"] = "#8caabf";
        material_params["navy"] = navy_params;

        nlohmann::json red_params;
        red_params["reflectance"] = "#ff0000";
        material_params["red"] = red_params;

        nlohmann::json green_params;
        green_params["reflectance"] = "#00ff00";
        material_params["green"] = green_params;

        nlohmann::json water_params;
        water_params["transparency"] = 1.0;
        water_params["ior"] = 1.8;
        material_params["water"] = water_params;


        nlohmann::json f9_emittance_json;
        f9_emittance_json["illuminant"] = "F9";
        f9_emittance_json["scale"] = 25;
        nlohmann::json f9_light_params;
        f9_light_params["reflectance"] = 1;
        f9_light_params["emittance"] = f9_emittance_json;
        material_params["F9_light"] = f9_light_params;


        render_params["materials"] = material_params;

        //surfaces
        nlohmann::json surfaces_params(nlohmann::json::value_t::array);

        for (const auto& object : objects_vector)
        {
            nlohmann::json obj_json;

            obj_json["type"] = "object";
            obj_json["material"] = material_list.at(object.material_type);
            obj_json["smooth"] = true;
            obj_json["position"] = object.position;
            obj_json["file"] = object.file_location.string();
            obj_json["rotation"] = object.rotation;
            obj_json["scale"] = object.scale.at(0);

            surfaces_params.insert(surfaces_params.end(), obj_json);
        }

        render_params["surfaces"] = surfaces_params;

        return render_params;
    }
}


