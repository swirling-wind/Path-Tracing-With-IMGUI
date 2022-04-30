#pragma once
#include "imgui.h"
#include "gui_param_tool.h"

namespace gui_render_widgets
{
    inline void show_render_params(gui_tools::shoot_params& shoot_photo_params)
    {
        ImGui::Checkbox("Use Photon Mapping or not", &shoot_photo_params.is_photon_map);
        ImGui::InputDouble("Emission photon number", &shoot_photo_params.photon_num, 1e6, 1e7, "%.0f");

        ImGui::Text("BVH type: ");
        ImGui::SameLine();
        if (ImGui::RadioButton("Octree", shoot_photo_params.is_octree))
        {
            shoot_photo_params.is_octree = true;
            shoot_photo_params.is_sah = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("SAH", shoot_photo_params.is_sah))
        {
            shoot_photo_params.is_octree = false;
            shoot_photo_params.is_sah = true;
        }

        // Image Params
        ImGui::InputFloat("Focal Length", &shoot_photo_params.focal_length, 1.0f, 1.0f, "%.2f");
        ImGui::InputFloat("Sensor Width", &shoot_photo_params.sensor_width, 1.0f, 1.0f, "%.2f");
        ImGui::InputFloat("Exposure compensation", &shoot_photo_params.exposure_compensation, 0.1f, 1.0f, "%.2f");
        ImGui::InputFloat("Gain Compensation", &shoot_photo_params.gain_compensation, 0.1f, 1.0f, "%.2f");

        // Camera Position: Eye & LookAt
        ImGui::Text("\nCamera");
        ImGui::InputFloat3("Eye Position", shoot_photo_params.camera_eye_pos.data());
        ImGui::InputFloat3("Look At", shoot_photo_params.camera_look_at.data());
    }

    inline void show_imported_objects(std::vector<gui_tools::object_imported>& objects_vector)
    {
        ImGui::Text("\nObjects");
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
            ImGui::TextUnformatted(gui_tools::material_list.at(iter->material_type));
            if (ImGui::BeginPopup(("select_popup" + index).c_str()))
            {
                ImGui::Text("Material");
                ImGui::Separator();
                for (int i = 0; i < gui_tools::num_of_material_list; i++)
                    if (ImGui::Selectable(gui_tools::material_list.at(i)))
                        iter->material_type = i;
                ImGui::EndPopup();
            }
        }

        // Remove the last imported object
        if (!objects_vector.empty())
        {
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                objects_vector.pop_back();
            }
        }
    }

    inline void show_add_object(const std::vector<std::filesystem::path>& obj_found_in_path, std::vector<gui_tools::object_imported>& objects_vector)
    {
        for (auto iter = obj_found_in_path.begin(); iter != obj_found_in_path.end(); ++iter)
        {
            const auto index = iter - obj_found_in_path.begin();
            if (ImGui::Button(iter->string().data()))
            {
                gui_tools::object_imported temp_object;
                temp_object.file_location = iter->filename();
                objects_vector.push_back(temp_object);
            }
        }
    }


}


