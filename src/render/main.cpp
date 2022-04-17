#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include <filesystem>
#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>
#include "camera/camera.h"
#include "common/option.h"
#include "common/util.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <array>
#include <iostream>
#include <thread>
#include <GLFW/glfw3.h> 

#include "gui_param_tool.h"
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1800, 900, "Path Tracing", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;

    // Render parameters =====================================================================
    gui_tools::shoot_params shoot_photo_params;
    //float exposure_compensation = -1.5;
    //float gain_compensation = 0.0;
    //int side_spp = 3; // to be powed
    //bool is_photon_map = false;
    //bool is_octree = true;
    //bool is_sah = false;

    std::array<int, 2> output_image_size = { 1280, 720 };

    std::vector<gui_tools::object_imported> objects_vector;

    std::array<float, 3>camera_eye_pos = { -0.2f, 2.2f, 10.0f };
    std::array<float, 3>camera_look_at = { -0.2f, 1.7f, 0.0f };

    constexpr int filename_max_length = 101;
    char file_save_name[filename_max_length] = "new image";

    //  =======================================================================================

    const std::filesystem::path model_path = std::filesystem::current_path() / "scenes";
    std::cout << "Display the obj files in path: " << model_path.string() << std::endl;
    const auto obj_found_in_path = gui_tools::get_models_in_folder(model_path);
    std::cout << obj_found_in_path.size() << std::endl;

    // Main loop    ============================================================================
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {

            ImGui::Begin("Path Tracing");
            // Render params
            ImGui::Checkbox("Use Photon Mapping or not", &shoot_photo_params.is_photon_map);
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


            // Camera Params

            ImGui::InputFloat("Focal Length", &shoot_photo_params.focal_length, 1.0f, 1.0f, "%.2f");
            ImGui::InputFloat("Sensor Width", &shoot_photo_params.sensor_width, 1.0f, 1.0f, "%.2f");
            ImGui::InputFloat("Exposure compensation", &shoot_photo_params.exposure_compensation, 0.1f, 1.0f, "%.2f");
            ImGui::InputFloat("Gain Compensation", &shoot_photo_params.gain_compensation, 0.1f, 1.0f, "%.2f");

            // Camera Position: Eye & LookAt
            ImGui::Text("Camera");
            ImGui::InputFloat3("Eye Position", camera_eye_pos.data());
            ImGui::InputFloat3("Look At", camera_look_at.data());

            // Display all imported objects
            ImGui::Text("\nObjects");
            for (auto iter = objects_vector.begin(); iter != objects_vector.end(); ++iter)
            {
                const auto index = std::to_string(iter - objects_vector.begin());

                ImGui::Text(("\n" + iter->file_location.string() + " " + index).c_str());
                ImGui::InputFloat3(("position " + index).c_str(), iter->position.data());

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


            // Press button to add a new object 
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


            ImGui::InputText("file name", file_save_name, IM_ARRAYSIZE(file_save_name));
            ImGui::InputInt("Side samples per pixel", &shoot_photo_params.side_spp);

            //Output gui_params as json
            if (ImGui::Button("\nOutput json"))
            {
                std::string save_name{ file_save_name };
                nlohmann::json test_json = generate_render_params(shoot_photo_params, save_name, camera_eye_pos, camera_look_at, output_image_size, objects_vector);

                std::ofstream out(save_name + ".json");
                out << test_json;

            }
            ImGui::SameLine();

            //Render
            if (ImGui::Button("\nStart offline Rendering in Current Setting"))
            {
                //TODO
                std::unique_ptr<Camera> camera;
                std::string save_name{ file_save_name };
                nlohmann::json render_json = generate_render_params(shoot_photo_params, save_name, camera_eye_pos, camera_look_at, output_image_size, objects_vector);

                try
                {
                    camera = std::make_unique<Camera>(render_json, shoot_photo_params.is_photon_map);
                }
                catch (const std::exception& ex)
                {
                    std::cout << ex.what() << std::endl;
                    return -1;
                }

                camera->capture();

            }

            //if (ImGui::Button("\ntestor"))
            //{
            //    std::cerr << shoot_photo_params.is_octree << " " << shoot_photo_params.is_sah << std::endl;
            //    std::cerr << shoot_photo_params.is_photon_map << std::endl;
            //}

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int test_render()
{
    std::cout << "Scene directory:" << std::endl << Scene::path.string() << std::endl << std::endl;


    std::vector<Option> options;
    try
    {
        options = availible(Scene::path);
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }

    if (options.empty())
    {
        std::cout << "No scenes found." << std::endl;
        return -1;
    }

    Option scene_option = getOption(options);

    std::ifstream scene_file(scene_option.path);
    nlohmann::json j;
    scene_file >> j;
    scene_file.close();

    std::unique_ptr<Camera> camera;
    try
    {
        camera = std::make_unique<Camera>(j, scene_option);
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }

    camera->capture();


    //camera->preview();

    return 0;
}
