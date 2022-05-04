#pragma once
#include "imgui.h"
#include "gui_param_tool.h"

namespace gui_widgets
{
    inline bool whether_able_to_render(
        const std::atomic<gui_params::render_status>& current_status,
        std::string& status_text,
        const std::vector<gui_params::object_imported>& objects_vector
    )
    {
        if (current_status != gui_params::render_status::awaiting)
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


    inline void show_render_params(gui_params::bvh_and_photon_params& integrator_params, gui_params::camera_params& shot_params)
    {
        // BVH and Photon
        ImGui::Checkbox("Use Photon Mapping or not", &integrator_params.is_photon_map);
        ImGui::InputDouble("Emission photon number", &integrator_params.photon_num, 1e6, 1e7, "%.0f");

        ImGui::Text("BVH type: ");
        ImGui::SameLine();
        if (ImGui::RadioButton("Octree", integrator_params.is_octree))
        {
            integrator_params.is_octree = true;
            integrator_params.is_sah = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("SAH", integrator_params.is_sah))
        {
            integrator_params.is_octree = false;
            integrator_params.is_sah = true;
        }

        
        // Image Params
        ImGui::InputFloat("Focal Length", &shot_params.focal_length, 1.0f, 1.0f, "%.2f");
        ImGui::InputFloat("Sensor Width", &shot_params.sensor_width, 1.0f, 1.0f, "%.2f");
        ImGui::InputFloat("Exposure compensation", &shot_params.exposure_compensation, 0.1f, 1.0f, "%.2f");
        ImGui::InputFloat("Gain Compensation", &shot_params.gain_compensation, 0.1f, 1.0f, "%.2f");

        // Camera Position: Eye & LookAt
        ImGui::Text("\nCamera");
        ImGui::InputFloat3("Eye Position", shot_params.camera_eye_pos.data());
        ImGui::InputFloat3("Look At", shot_params.camera_look_at.data());
    }

    inline void show_imported_objects(std::vector<gui_params::object_imported>& objects_vector)
    {
        ImGui::Text("\nObjects");
        int delete_object_index = -1;
        for (auto iter = objects_vector.begin(); iter != objects_vector.end(); ++iter)
        {
            const auto index = std::to_string(iter - objects_vector.begin());

            ImGui::Text(("\n" + iter->file_location.string() + " " + index).c_str());
            ImGui::InputFloat3(("position " + index).c_str(), iter->position.data());
            ImGui::InputFloat3(("rotation " + index).c_str(), iter->rotation.data());
            ImGui::SameLine();
            ImGui::InputFloat(("scale " + index).c_str(), iter->scale.data(), 0.1f);

            if (ImGui::Button(("Select " + index + "'s material ..").c_str()))
                ImGui::OpenPopup(("select_popup" + index).c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted(gui_params::material_list.at(iter->material_type));
            if (ImGui::BeginPopup(("select_popup" + index).c_str()))
            {
                ImGui::Text("Material");
                ImGui::Separator();
                for (int i = 0; i < gui_params::num_of_material_list; i++)
                    if (ImGui::Selectable(gui_params::material_list.at(i)))
                        iter->material_type = i;
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button(("Remove " + index).c_str()))
            {
                delete_object_index = iter - objects_vector.begin();
            }
        }

        if (delete_object_index >= 0)
        {
            objects_vector.erase(objects_vector.begin() + delete_object_index);
        }
    }

    inline void show_available_objects(const std::vector<std::filesystem::path>& obj_found_in_path, std::vector<gui_params::object_imported>& objects_vector)
    {
        for (auto iter = obj_found_in_path.begin(); iter != obj_found_in_path.end(); ++iter)
        {
            const auto index = iter - obj_found_in_path.begin();
            if (ImGui::Button(iter->string().data()))
            {
                gui_params::object_imported temp_object;
                temp_object.file_location = iter->filename();
                objects_vector.push_back(temp_object);
            }
        }
    }


}


