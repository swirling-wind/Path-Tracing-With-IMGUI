#pragma once
#include "imgui.h"
#include "gui_param_tool.h"

namespace gui_widgets
{
    inline void choose_property_file(const std::vector<std::filesystem::path>& property_vec,
        camera_space::camera_params& shot_params,
        integrator_space::bvh_and_photon_params& integrator_params,
        material_space::material_map& materials_map,
        object_space::object_list& objects_vector
    )
    {
        if (ImGui::Button("Import property file"))
        {
            ImGui::OpenPopup("Import property");
        }
        if (ImGui::BeginPopup("Import property", ImGuiWindowFlags_MenuBar))
        {
            ImGui::Text("Properties can be imported:");
            for (auto iter = property_vec.begin(); iter != property_vec.end(); ++iter)
            {
                const auto index = iter - property_vec.begin();
                if (ImGui::Button(iter->filename().string().data()))
                {
                    std::ifstream scene_file(gui_constant_params::property_file_path / iter->filename());
                    nlohmann::json total_properties;
                    scene_file >> total_properties;
                    scene_file.close();
                    
                    gui_params_space::get_all_params_from_properties(total_properties, shot_params, integrator_params, materials_map, objects_vector);
                }
            }   
            ImGui::EndPopup();
        }
    }


    inline bool whether_able_to_render(
        const std::atomic<gui_params_space::render_status>& current_status,
        std::string& status_text,
        const object_space::object_list& objects_vector
    )
    {
        if (current_status != gui_params_space::render_status::awaiting)
        {
            status_text = "Renderer is busy rendering ...";
            return false;
        }
        if (objects_vector.empty())
        {
            status_text = "No object to render ...";
            return false;
        }
        return true;
    }

    inline void show_material_manager(material_space::material_map& material_map)
    {
        if (ImGui::Button("Materials Management"))
        {
            ImGui::OpenPopup("Manage materials");
        }
        if (ImGui::BeginPopup("Manage materials", ImGuiWindowFlags_MenuBar))
        {
            if (material_map.empty())
            {
                ImGui::Text("No material ...");
            }

            material_space::material_map::iterator to_delete_material = material_map.end();
            for (auto iter = material_map.begin(); iter != material_map.end(); ++iter)
            {
                if (iter->first == "default")
                {
                    continue;
                }

                const auto m_name = iter->first;
                ImGui::Text(m_name.c_str());
                if (ImGui::TreeNode(("Properties##" + m_name).c_str()))
                {
                    ImGui::SliderFloat(("Roughness##" + m_name).c_str(), &iter->second.roughness, 0.0f, 1.0f);
                    ImGui::SliderFloat(("Specular Roughness##" + m_name).c_str(), &iter->second.specular_roughness, 0.0f, 1.0f);
                    ImGui::SliderFloat(("Transparency##" + m_name).c_str(), &iter->second.transparency, 0.0f, 1.0f);
                    ImGui::ColorEdit3(("Reflectance##" + m_name).c_str(), iter->second.reflectance.data());
                    ImGui::ColorEdit3(("Specular Reflectance##" + m_name).c_str(), iter->second.specular_reflectance.data());
                    ImGui::ColorEdit3(("Transmittance##" + m_name).c_str(), iter->second.transmittance.data());

                    ImGui::Checkbox(("Is Perfect mirror##" + m_name).c_str(), &iter->second.is_perfect_mirror);
                    ImGui::SameLine();
                    ImGui::Checkbox(("Is Emitting##" + m_name).c_str(), &iter->second.emittance.is_emit);

                    if (iter->second.emittance.is_emit)
                    {
                        ImGui::ColorEdit3(("Emittance Color##" + m_name).c_str(), iter->second.emittance.emittance_color.data());
                        ImGui::SliderFloat(("Emittance Scale##" + m_name).c_str(), &iter->second.emittance.emittance_scale, 100.0f, 900.0f);
                    }
                    if (ImGui::RadioButton(("No refract##" + m_name).c_str(), iter->second.ior.no_need_ior))
                    {
                        iter->second.ior.no_need_ior = true;
                        iter->second.ior.is_simple_ior = false;
                        iter->second.ior.is_complex_ior = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton(("Simple IOR##" + m_name).c_str(), iter->second.ior.is_simple_ior))
                    {
                        iter->second.ior.no_need_ior = false;
                        iter->second.ior.is_simple_ior = true;
                        iter->second.ior.is_complex_ior = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton(("Complex IOR##" + m_name).c_str(), iter->second.ior.is_complex_ior))
                    {
                        iter->second.ior.no_need_ior = false;
                        iter->second.ior.is_simple_ior = false;
                        iter->second.ior.is_complex_ior = true;
                    }
                    if (iter->second.ior.is_simple_ior)
                    {
                        ImGui::InputFloat(("Simple IOR##" + m_name).c_str(), &iter->second.ior.simple_ior, 0.1f, 0.2f);
                    }
                    if (iter->second.ior.is_complex_ior)
                    {
                        ImGui::InputFloat3((" Real IOR ##" + m_name).c_str(), iter->second.ior.real_ior.data());
                        ImGui::InputFloat3(("Imaginary IOR##" + m_name).c_str(), iter->second.ior.imaginary_ior.data());
                    }
                    ImGui::TreePop();
                }

                if (ImGui::Button(("Remove##" + m_name).c_str()))
                {
                    to_delete_material = iter;
                }
                ImGui::NewLine();
            }

            if (to_delete_material != material_map.end())
            {
                material_map.erase(to_delete_material);
            }
            ImGui::EndPopup();
        }
    }

    inline void add_new_material(const char material_name[], material_space::material_params& new_material, material_space::material_map& materials_map)
    {
        std::string new_material_name = std::string{ material_name };
        ImGui::SliderFloat("Roughness", &new_material.roughness, 0.0f, 1.0f);
        ImGui::ColorEdit3("Reflectance", new_material.reflectance.data());

        if (ImGui::Button(" Add "))
        {
            materials_map.emplace(new_material_name, new_material);
            new_material = material_space::material_params{};
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
    }
    
    inline void show_integrator_params(integrator_space::bvh_and_photon_params& integrator_params)
    {
        // BVH and Photon
        ImGui::Text("Integrator: ");
        ImGui::SameLine();
        if (ImGui::RadioButton("Path Tracing", integrator_params.is_path_tracing))
        {
            integrator_params.is_path_tracing = true;
            integrator_params.is_photon_map = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Photon Mapping", integrator_params.is_photon_map))
        {
            integrator_params.is_path_tracing = false;
            integrator_params.is_photon_map = true;

        }
        if (integrator_params.is_photon_map)
        {
            ImGui::InputFloat("Emission photon number", &integrator_params.photon_num, 1e6, 1e7, "%.0f");
            ImGui::InputFloat("Caustic photon multiplier", &integrator_params.caustic_multiplier, 1.0, 2.0, "%.0f");
            integrator_params.photon_num = std::clamp(integrator_params.photon_num, 1e6f, 9e8f);
            integrator_params.caustic_multiplier = std::clamp(integrator_params.caustic_multiplier, 1.0f, 50.0f);
        }

        ImGui::Text("BVH type: ");
        ImGui::SameLine();
        if (ImGui::RadioButton("Octree", integrator_params.is_octree))
        {
            integrator_params.is_octree = true;
            integrator_params.is_quaternary_sah = false;
            integrator_params.is_binary_sah = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Quaternary SAH", integrator_params.is_quaternary_sah))
        {
            integrator_params.is_octree = false;
            integrator_params.is_quaternary_sah = true;
            integrator_params.is_binary_sah = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Binary SAH", integrator_params.is_binary_sah))
        {
            integrator_params.is_octree = false;
            integrator_params.is_quaternary_sah = false;
            integrator_params.is_binary_sah = true;
        }
        if (integrator_params.is_quaternary_sah || integrator_params.is_binary_sah)
        {
            ImGui::InputInt("Slabs number", &integrator_params.bins_per_axis, 1, 4);
            integrator_params.bins_per_axis = std::clamp(integrator_params.bins_per_axis, 2, 32);
        }

    }

    inline void show_camera_params(camera_space::camera_params& shot_params)
    {
        ImGui::Text("Tone mapping: ");
        ImGui::SameLine();
        if (ImGui::RadioButton("ACES", shot_params.is_aces_tone_mapper))
        {
            shot_params.is_aces_tone_mapper = true;
            shot_params.is_hable_tone_mapper = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Hable", shot_params.is_hable_tone_mapper))
        {
            shot_params.is_aces_tone_mapper = false;
            shot_params.is_hable_tone_mapper = true;
        }

        ImGui::Text("\nImage");
        ImGui::InputFloat("Focal Length", &shot_params.focal_length, 1.0f, 1.0f, "%.2f");
        ImGui::InputFloat("Sensor Width", &shot_params.sensor_width, 1.0f, 1.0f, "%.2f");
        ImGui::InputFloat("Exposure compensation", &shot_params.exposure_compensation, 0.1f, 1.0f, "%.2f");
        ImGui::InputFloat("Gain Compensation", &shot_params.gain_compensation, 0.1f, 1.0f, "%.2f");
        ImGui::InputInt("Side samples per pixel", &shot_params.side_spp, 1, 5);
        shot_params.side_spp = std::clamp(shot_params.side_spp, 1, 100);
                
        ImGui::Text("Camera");
        ImGui::InputFloat3("Eye Position", shot_params.camera_eye_pos.data());
        ImGui::InputFloat3("Look At", shot_params.camera_look_at.data());
        
    }

    inline void show_imported_objects(object_space::object_list& objects_vector, const material_space::material_map& materials_map)
    {
        if (objects_vector.empty())
        {
            ImGui::Text("No objects now");
        }

        ImGui::Text("\nObjects");
        int delete_object_index = -1;
        for (auto iter = objects_vector.begin(); iter != objects_vector.end(); ++iter)
        {
            const auto index = std::to_string(iter - objects_vector.begin());

            ImGui::Text(("[" + index + "] " + iter->obj_file_name).c_str());            
            ImGui::InputFloat3(("position##" + index).c_str(), iter->position.data());
            ImGui::InputFloat3(("rotation##" + index).c_str(), iter->rotation.data());
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
            ImGui::InputFloat(("scale##" + index).c_str(), iter->scale.data(), 0.1f);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.7f, 0.7f));
            if (ImGui::Button(("Material: " + iter->material_type + "##" + index).c_str()))
            {
                ImGui::OpenPopup(("choose_material##" + index).c_str());
            }
            ImGui::PopStyleColor(2);

            if (ImGui::BeginPopup(("choose_material##" + index).c_str(), ImGuiWindowFlags_MenuBar))
            {
                for (const auto& material : materials_map)
                {
                    if (ImGui::Button(material.first.c_str()))
                    {
                        iter->material_type = material.first;
                    }
                }
                ImGui::EndPopup();
            }

            if (ImGui::Button(("Remove##" + index).c_str()))
            {
                delete_object_index = iter - objects_vector.begin();
            }
            ImGui::NewLine();
        }

        if (delete_object_index >= 0)
        {
            objects_vector.erase(objects_vector.begin() + delete_object_index);
        }
    }

    inline void show_available_objects(const std::vector<std::filesystem::path>& obj_name_found_in_path, object_space::object_list& obj_vec)
    {
        if (ImGui::Button("Add new object.."))
        {
            ImGui::OpenPopup("add_object");
        }
        if (ImGui::BeginPopup("add_object"))
        {
            for (const auto& obj : obj_name_found_in_path)
            {
                const std::string obj_file_name = obj.filename().string();
                if (ImGui::Button(obj_file_name.c_str()))
                {
                    object_space::object_with_material to_add_object;
                    to_add_object.obj_file_name = obj_file_name;
                    obj_vec.emplace_back(to_add_object);
                }
            }
            ImGui::EndPopup();
        }
    }


}


