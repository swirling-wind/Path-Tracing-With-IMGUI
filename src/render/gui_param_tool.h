#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>

#include "color/illuminant.h"
#include "color/srgb.h"
#include "common/util.h"

namespace gui_constant_params
{
    const std::filesystem::path obj_file_path = std::filesystem::current_path() / "objects";
    const std::filesystem::path property_file_path = std::filesystem::current_path() / "properties";
}

namespace camera_space
{
    struct camera_params
    {
        float focal_length = 50;
        float sensor_width = 35;
        std::array<float, 3>camera_eye_pos = { 0.0f, 1.5f, -8.0f };
        std::array<float, 3>camera_look_at = { 0.0f, 1.5f, 0.0f };
        int side_spp = 3;

        std::array<int, 2> output_image_size = { 1280, 720 };
        float exposure_compensation = -1.5;
        float gain_compensation = 0.0;
        bool is_aces_tone_mapper = true;
        bool is_hable_tone_mapper = false;
    };

    inline void from_json(const nlohmann::json& total_properties, camera_params& imported_camera_params)
    {
        const nlohmann::json& camera_properties = total_properties.at("cameras").at(0);
        imported_camera_params.focal_length = camera_properties.at("focal_length");
        imported_camera_params.sensor_width = camera_properties.at("sensor_width");
        imported_camera_params.camera_eye_pos = camera_properties.at("eye");
        imported_camera_params.camera_look_at = camera_properties.at("look_at");
        imported_camera_params.side_spp = camera_properties.at("sqrtspp");

        const nlohmann::json& image_properties = camera_properties.at("image");
    /*    imported_camera_params.output_image_size.at(0) = image_properties.at("width");
        imported_camera_params.output_image_size.at(1) = image_properties.at("height");*/
        imported_camera_params.exposure_compensation = getOptional(image_properties, "exposure_compensation", -1.0);
        imported_camera_params.gain_compensation = getOptional(image_properties, "gain_compensation", 0.0);

        const std::string tone_mapper = getOptional<std::string>(image_properties, "tonemapper", "ACES");
        if (tone_mapper == "ACES")
        {
            imported_camera_params.is_aces_tone_mapper = true;
            imported_camera_params.is_hable_tone_mapper = false;
        }
        else
        {
            imported_camera_params.is_aces_tone_mapper = false;
            imported_camera_params.is_hable_tone_mapper = true;
        }
    }

    inline void to_json(nlohmann::json& render_properties_for_camera, const camera_params& shot_params)
    {
        //camera
        nlohmann::json camera_params(nlohmann::json::value_t::array);
        nlohmann::json new_camera_instance;
        new_camera_instance["focal_length"] = shot_params.focal_length;
        new_camera_instance["sensor_width"] = shot_params.sensor_width;
        new_camera_instance["eye"] = shot_params.camera_eye_pos;
        new_camera_instance["look_at"] = shot_params.camera_look_at;

        //Image
        nlohmann::json image_params;
        image_params["width"] = shot_params.output_image_size.at(0);
        image_params["height"] = shot_params.output_image_size.at(1);
        image_params["exposure_compensation"] = shot_params.exposure_compensation;
        image_params["gain_compensation"] = shot_params.gain_compensation;
        image_params["tonemapper"] = shot_params.is_aces_tone_mapper ? "ACES" : "HABLE";
        new_camera_instance["image"] = image_params;

        new_camera_instance["sqrtspp"] = shot_params.side_spp;
        new_camera_instance["savename"] = "preview";

        camera_params.insert(camera_params.end(), new_camera_instance);
        render_properties_for_camera["cameras"] = camera_params;
    }
}

namespace integrator_space
{
    struct bvh_and_photon_params
    {
        bool is_path_tracing = true;
        bool is_photon_map = false;
        float photon_num = 1e6;
        float caustic_multiplier = 10.0;

        bool is_octree = true;
        bool is_quaternary_sah = false;
        bool is_binary_sah = false;
        int bins_per_axis = 8;
    };

    inline void from_json(const nlohmann::json& total_properties, bvh_and_photon_params& imported_integrator_params)
    {
        imported_integrator_params.is_photon_map = (total_properties.find("photon_map") != total_properties.end());
        imported_integrator_params.is_path_tracing = !(imported_integrator_params.is_photon_map);
        if (imported_integrator_params.is_photon_map)
        {
            imported_integrator_params.photon_num = total_properties.at("photon_map").at("emissions");
            imported_integrator_params.caustic_multiplier = total_properties.at("photon_map").at("caustic_factor");
        }

        std::string type = getOptional<std::string>(total_properties.at("bvh"), "type", "OCTREE");
        std::transform(type.begin(), type.end(), type.begin(), toupper);
        if (type == "OCTREE")
        {
            imported_integrator_params.is_octree = true;
            imported_integrator_params.is_quaternary_sah = false;
            imported_integrator_params.is_binary_sah = false;
        }
        else
        {
            imported_integrator_params.bins_per_axis = total_properties.at("bvh").at("bins_per_axis");
            if (type == "QUATERNARY_SAH")
            {
                imported_integrator_params.is_octree = false;
                imported_integrator_params.is_quaternary_sah = true;
                imported_integrator_params.is_binary_sah = false;
            }
            else
            {
                imported_integrator_params.is_octree = false;
                imported_integrator_params.is_quaternary_sah = false;
                imported_integrator_params.is_binary_sah = true;
            }
        }
    }

    inline void to_json(nlohmann::json& render_properties_for_bvh_and_photon, const bvh_and_photon_params& integrator_params)
    {
        render_properties_for_bvh_and_photon["num_render_threads"] = -1; // All

        nlohmann::json bvh_type;
        if (integrator_params.is_octree)
        {
            bvh_type["type"] = "octree";
        }
        else
        {
            bvh_type["bins_per_axis"] = integrator_params.bins_per_axis;
            if (integrator_params.is_quaternary_sah)
            {
                bvh_type["type"] = "quaternary_sah";
            }
            else
            {
                bvh_type["type"] = "binary_sah";
            }
        }
        render_properties_for_bvh_and_photon["bvh"] = bvh_type;

        if (integrator_params.is_photon_map)
        {
            nlohmann::json photon_map_json;
            photon_map_json["emissions"] = floor(integrator_params.photon_num);
            photon_map_json["caustic_factor"] = integrator_params.caustic_multiplier;
            photon_map_json["k_nearest_photons"] = 50;
            photon_map_json["max_photons_per_octree_leaf"] = 200;
            photon_map_json["direct_visualization"] = false;
            render_properties_for_bvh_and_photon["photon_map"] = photon_map_json;
        }
    }
}

namespace material_space
{
    using float3_array = std::array<float, 3>;

    inline void get_reflectance_between_0_1(const nlohmann::json& j, const std::string& field, float3_array& reflectance)
    {
        if (j.find(field) != j.end())
        {
            const nlohmann::json& r = j.at(field);
            if (r.type() == nlohmann::json::value_t::string)
            {
                std::string hex_string = r.get<std::string>();
                if (hex_string.size() == 7 && hex_string[0] == '#')
                {
                    hex_string.erase(0, 1);
                    std::stringstream ss;
                    ss << std::hex << hex_string;

                    uint32_t color_int;
                    ss >> color_int;

                    //[0,1]
                    reflectance = float3_array{ static_cast<float>((color_int >> 16) & 0xFF) / 255.0f, static_cast<float>((color_int >> 8) & 0xFF) / 255.0f, static_cast<float>(color_int & 0xFF) / 255.0f };
                }
            }
            else if (r.type() == nlohmann::json::value_t::array)
            {
                //[0,1]
                reflectance = r.get<float3_array>();
            }
        }
    };
    
    inline float get_scale_from_total_emittance(const glm::dvec3 total_emittance)
    {
        double scale = 100.0;
        const glm::dvec3 temp_emittance{ total_emittance };
        while (temp_emittance.r / scale >= 1.0 || temp_emittance.g / scale >= 1.0  || temp_emittance.b / scale >= 1.0)
        {
            scale += 100.0;
        }
        std::cerr << "Scale: " << scale << std::endl;
        return static_cast<float>(scale);
    }

    struct material_params
    {
        struct ior_param
        {
            bool no_need_ior = true;

            bool is_simple_ior = false;
            float simple_ior = 1.0f;

            bool is_complex_ior = false;
            float3_array real_ior = { 0.0f,0.0f,0.0f };
            float3_array imaginary_ior = { 0.0f,0.0f,0.0f };
        };

        struct emit_param
        {
            bool is_emit = false;
            float3_array emittance_color = { 1.0f,1.0f,1.0f };  //[0,1]
            float emittance_scale = { 100.0f };
        };
        
        float roughness = 0.0f;
        float specular_roughness = 0.0f;
        float transparency = 0.0f;

        //[0,1]
        float3_array reflectance = {1.0f,1.0f,1.0f};
        float3_array specular_reflectance = { 1.0f,1.0f,1.0f };
        float3_array transmittance = { 1.0f,1.0f,1.0f };

        bool is_perfect_mirror = false;
        emit_param emittance;

        ior_param ior;
    };

    using material_map = std::unordered_map<std::string, material_params>;
    
    inline void from_json(const nlohmann::json& material_properties, material_params& material_param)
    {
        getToOptional(material_properties, "roughness", material_param.roughness);
        getToOptional(material_properties, "specular_roughness", material_param.specular_roughness);
        getToOptional(material_properties, "transparency", material_param.transparency);
        getToOptional(material_properties, "perfect_mirror", material_param.is_perfect_mirror);

        // color range [0,1]
        get_reflectance_between_0_1(material_properties, "reflectance", material_param.reflectance);
        get_reflectance_between_0_1(material_properties, "specular_reflectance", material_param.specular_reflectance);
        get_reflectance_between_0_1(material_properties, "transmittance", material_param.transmittance);

        material_param.reflectance = sRGB::gammaExpand(material_param.reflectance);

        if (material_properties.find("emittance") != material_properties.end())
        {//TODO: Temp for Transition
            material_param.emittance.is_emit = true;
            glm::dvec3 temp_total_emittance{ 1.0 };

            const auto emittance_property = material_properties.at("emittance");
            if (emittance_property.type() == nlohmann::json::value_t::object)
            {
                const double scale = getOptional(emittance_property, "scale", 1.0);
                const double temperature = getOptional<double>(emittance_property, "temperature", -1.0);
                if (temperature > 0.0)
                {
                    temp_total_emittance = sRGB::RGB(CIE::Illuminant::blackbody(temperature) * scale);
                }
                else
                {
                    std::string illuminant = getOptional<std::string>(emittance_property, "illuminant", "D65");
                    std::transform(illuminant.begin(), illuminant.end(), illuminant.begin(), toupper);
                    temp_total_emittance = sRGB::RGB(CIE::Illuminant::whitePoint(illuminant.c_str()) * scale);
                }
            }
            else
            {
                temp_total_emittance = emittance_property.get<glm::dvec3>();
            }
            material_param.emittance.emittance_scale = get_scale_from_total_emittance(temp_total_emittance);
            material_param.emittance.emittance_color = float3_array{
                static_cast<float>(temp_total_emittance.r) / material_param.emittance.emittance_scale,
                static_cast<float>(temp_total_emittance.g) / material_param.emittance.emittance_scale,
                static_cast<float>(temp_total_emittance.b) / material_param.emittance.emittance_scale
            };
        }
        else if (material_properties.find("light_emittance") != material_properties.end())
        {
            material_param.emittance.is_emit = true;
            const nlohmann::json emittance_property = material_properties.at("light_emittance");
            material_param.emittance.emittance_scale = getOptional(emittance_property, "emittance_scale", 100.0f);
            material_param.emittance.emittance_color = getOptional(emittance_property, "emittance_color", float3_array{ 1.0f, 1.0f, 1.0f });
        }

        if (material_properties.find("ior") != material_properties.end())
        {
            material_param.ior.no_need_ior = false;
            const auto ior_property = material_properties.at("ior");
            if (ior_property.type() == nlohmann::json::value_t::object)
            {
                material_param.ior.is_simple_ior = false;
                material_param.ior.is_complex_ior = true;
                material_param.ior.imaginary_ior = getOptional(ior_property, "imaginary", float3_array{ 0.0f, 0.0f, 0.0f });
                material_param.ior.real_ior = getOptional(ior_property, "real", float3_array{ 0.0f, 0.0f, 0.0f });

            }
            else
            {
                material_param.ior.is_simple_ior = true;
                material_param.ior.is_complex_ior = false;
                ior_property.get_to(material_param.ior.simple_ior);
            }
        }
        else
        {
            material_param.ior.no_need_ior = true;
        }
    }

    inline void to_json(nlohmann::json& material_properties, const material_params& material_param)
    {
        material_properties["roughness"] = material_param.roughness;
        material_properties["specular_roughness"] = material_param.specular_roughness;
        material_properties["transparency"] = material_param.transparency;
        material_properties["perfect_mirror"] = material_param.is_perfect_mirror;
        material_properties["reflectance"] = float3_array{ material_param.reflectance.at(0), material_param.reflectance.at(1), material_param.reflectance.at(2) };
        material_properties["specular_reflectance"] =  float3_array{ material_param.specular_reflectance.at(0), material_param.specular_reflectance.at(1), material_param.specular_reflectance.at(2) };
        material_properties["transmittance"] = float3_array{ material_param.transmittance.at(0), material_param.transmittance.at(1), material_param.transmittance.at(2) };

        if (material_param.emittance.is_emit)
        {
            nlohmann::json light_emittance;
            const float scale = material_param.emittance.emittance_scale;
            material_properties["emittance"] = {
                material_param.emittance.emittance_color.at(0) * scale,
                material_param.emittance.emittance_color.at(1) * scale,
                material_param.emittance.emittance_color.at(2) * scale,
            };
        }
        if (!material_param.ior.no_need_ior)
        {
            if (material_param.ior.is_simple_ior)
            {
                material_properties["ior"] = material_param.ior.simple_ior;
            }
            else if (material_param.ior.is_complex_ior)
            {
                nlohmann::json complex_ior;
                complex_ior["real"] = { material_param.ior.real_ior.at(0), material_param.ior.real_ior.at(1), material_param.ior.real_ior.at(2) };
                complex_ior["imaginary"] = { material_param.ior.imaginary_ior.at(0), material_param.ior.imaginary_ior.at(1), material_param.ior.imaginary_ior.at(2) };
                material_properties["ior"] = complex_ior;
            }
        }
       
    }

    inline void to_json(nlohmann::json& total_properties, const material_map& materials_map)
    {
        nlohmann::json materials_properties;
        for (const auto& single_material_property : materials_map)
        {
            materials_properties[single_material_property.first] = single_material_property.second;
        }

        total_properties["materials"] = materials_properties;
    }
}

namespace object_space
{
    using float3_array = std::array<float, 3>;

    struct object_with_material
    {
        bool is_smooth = true;
        std::string obj_file_name;
        std::string material_type;
        float3_array position{ 0.0,0.0,0.0 };
        float3_array rotation{ 0.0,0.0,0.0 };
        std::array<float, 1> scale{ 1.0 };
    };

    using object_list = std::vector<object_with_material>;

    inline void from_json(const nlohmann::json& total_properties, object_list& objects_vector)
    {
        nlohmann::json objects_properties = total_properties.at("surfaces");
        for (const auto& each_object : objects_properties)
        {
            std::string type = each_object.at("type");
            if (type != "object")
            {
                continue;
            }

            object_with_material added_object;
            added_object.material_type = getOptional(each_object, "material", std::string{ "default" });
            added_object.position = getOptional(each_object, "position", float3_array{0.0f,0.0f,0.0f});
            added_object.rotation = getOptional(each_object, "rotation", float3_array{0.0f,0.0f,0.0f});
            added_object.scale = std::array<float, 1>{getOptional(each_object, "scale", 1.0f)};
            added_object.is_smooth = getOptional(each_object, "smooth", true);
            added_object.obj_file_name = getOptional(each_object, "file", std::string{});
            objects_vector.emplace_back(added_object);
        }
    }

    inline void to_json(nlohmann::json& object_properties, const object_list& objects_vector)
    {
        nlohmann::json surfaces_params(nlohmann::json::value_t::array);

        for (const auto& object : objects_vector)
        {
            nlohmann::json obj_json;

            obj_json["type"] = "object";
            obj_json["material"] = object.material_type;
            obj_json["smooth"] = object.is_smooth;
            obj_json["position"] = object.position;
            obj_json["file"] = object.obj_file_name;
            obj_json["rotation"] = object.rotation;
            obj_json["scale"] = object.scale.at(0);

            surfaces_params.insert(surfaces_params.end(), obj_json);
        }
        object_properties["surfaces"] = surfaces_params;
    }
}
    


namespace gui_params_space
{
    inline void get_all_params_from_properties(const nlohmann::json& total_properties,
        camera_space::camera_params& shot_params,
        integrator_space::bvh_and_photon_params& integrator_params,
        material_space::material_map& materials_map,
        object_space::object_list& objects_vector
    )
    {
        shot_params = total_properties.get<camera_space::camera_params>();
        integrator_params = total_properties.get<integrator_space::bvh_and_photon_params>();
        materials_map = total_properties.at("materials").get<material_space::material_map>();
        objects_vector = total_properties.get<object_space::object_list>();
    }

    inline nlohmann::json generate_all_properties(
        const camera_space::camera_params& shot_params,
        const integrator_space::bvh_and_photon_params& integrator_params,
        const material_space::material_map& materials_map,
        const object_space::object_list& objects_vector
    )
    {
        nlohmann::json total_properties = shot_params;
        total_properties.update(integrator_params);
        total_properties.update(materials_map);
        total_properties.update(objects_vector);
        return total_properties;
    }

    inline nlohmann::json generate_both_integrator_and_material_objects_properties(
        const integrator_space::bvh_and_photon_params& integrator_params,
        const material_space::material_map& materials_map,
        const object_space::object_list& objects_vector

    )
    {
        nlohmann::json integrator_and_scene_properties = integrator_params;
        integrator_and_scene_properties.update(materials_map);
        integrator_and_scene_properties.update(objects_vector);
        return integrator_and_scene_properties;
    }



    enum class render_status : int {
        awaiting,

        busy_initing_scene,

        scene_prepared_ready_to_preview,

        busy_rendering,
        finished_preview,

        rendering_for_offline,
        finished_offline
    };

    constexpr int num_of_material_list = 18;
    inline std::array<const char*, num_of_material_list> material_list = {
        "default", "light",  "wood", "walnut", "oak",
        "gold", "nickel", "copper", "iron", "brass",
        "palladium", "glass",

        "shadow_green", "navy", "red", "green",

        "water", "F9_light"
    };

    struct object_imported
    {
        std::array<float, 3> position{ 0.0,0.0,0.0 };
        std::filesystem::path file_location;
        int material_type{ 0 };
        std::array<float, 3> rotation{ 0.0,0.0,0.0 };
        std::array<float, 1> scale{ 1.0 };
    };

    inline std::vector<std::filesystem::path> get_files_in_folder(const std::filesystem::path& folder_path, std::string postfix)
    {
        std::vector<std::filesystem::path> file_path_vector;
        for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(folder_path))
        {
            if (!file.path().has_extension() || file.path().extension() != postfix)
                continue;
            file_path_vector.push_back(file.path());
            std::string obj_file_name = file.path().filename().string();
            std::cout << obj_file_name << std::endl;
        }
        return file_path_vector;
    }

    inline void load_external_files(
        std::vector<std::filesystem::path>& obj_found_in_path,
        std::vector<std::filesystem::path>& properties_found_in_path)
    {
        std::cout << "Display the .obj files in path: " << gui_constant_params::obj_file_path.string() << std::endl;
        obj_found_in_path = gui_params_space::get_files_in_folder(gui_constant_params::obj_file_path, ".obj");
        std::cout << obj_found_in_path.size() << std::endl;

        //  =======================================================================================
        std::cout << "\nDisplay the properties files in path: " << gui_constant_params::property_file_path.string() << std::endl;
        properties_found_in_path = gui_params_space::get_files_in_folder(gui_constant_params::property_file_path, ".json");
        std::cout << properties_found_in_path.size() << std::endl;
    }

    inline nlohmann::json generate_material_and_object_properties(const std::vector<object_imported>& objects_vector)
    {
        nlohmann::json render_properties_for_material_and_objects;
        render_properties_for_material_and_objects["ior"] = 1;

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
        glass_params["transparency"] = 1.0;
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


        render_properties_for_material_and_objects["materials"] = material_params;

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

        render_properties_for_material_and_objects["surfaces"] = surfaces_params;

        return render_properties_for_material_and_objects;
    }



    inline nlohmann::json generate_total_render_properties(
        const nlohmann::json& render_properties_for_camera,
        const nlohmann::json& render_properties_for_bvh_and_photon,
        const nlohmann::json& render_properties_for_material_and_objects
    )
    {
        nlohmann::json render_properties;

        // Camera
        render_properties.update(render_properties_for_camera);

        // BVH and photon
        render_properties.update(render_properties_for_bvh_and_photon);

        // Materials and surfaces
        render_properties.update(render_properties_for_material_and_objects);
        return render_properties;
    }

    inline nlohmann::json generate_total_render_properties(
        const camera_space::camera_params& shot_params,
        const integrator_space::bvh_and_photon_params& integrator_params,
        const std::vector<object_imported>& objects_vector
    )
    {
        nlohmann::json render_properties;

        // Camera
        const nlohmann::json render_properties_for_camera = shot_params;
        render_properties.update(render_properties_for_camera);

        // BVH and photon
        const nlohmann::json render_properties_for_bvh_and_photon = integrator_params;
        render_properties.update(render_properties_for_bvh_and_photon);

        // Materials and surfaces
        const nlohmann::json render_properties_for_material_and_objects = generate_material_and_object_properties(objects_vector);
        render_properties.update(render_properties_for_material_and_objects);

        return render_properties;
    }

    inline nlohmann::json generate_total_render_properties(
        const camera_space::camera_params& shot_params, const std::string save_name,
        const integrator_space::bvh_and_photon_params& integrator_params,
        const std::vector<object_imported>& objects_vector
    )
    {
        nlohmann::json render_properties;

        // Camera
        const nlohmann::json render_properties_for_camera = shot_params;
        render_properties.update(render_properties_for_camera);

        render_properties["cameras"]["savename"] = save_name;

        // BVH and photon
        const nlohmann::json render_properties_for_bvh_and_photon = integrator_params;
        render_properties.update(render_properties_for_bvh_and_photon);

        // Materials and surfaces
        const nlohmann::json render_properties_for_material_and_objects = generate_material_and_object_properties(objects_vector);
        render_properties.update(render_properties_for_material_and_objects);

        return render_properties;
    }
}


