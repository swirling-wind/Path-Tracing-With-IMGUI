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

#include <glad/glad.h>
#include <nlohmann/json.hpp>
#define STB_IMAGE_IMPLEMENTATION    
#include <stb/stb_image.h>

#include "camera/camera.h"
#include "common/option.h"
#include "common/util.h"
#include "gui_param_tool.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <array>
#include <iostream>
#include <thread>
#include <GLFW/glfw3.h> 

#include "gui_render_widgets.h"

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    const char* glsl_version = "#version 130";

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Path Tracing", nullptr, nullptr);

    // Set window icon
    int icon_width, icon_height, icon_channels;
    unsigned char* img = stbi_load("icon.png", &icon_width, &icon_height, &icon_channels, 4);
    GLFWimage icon { icon_width,icon_height,img };
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(img);

    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    //Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("GLAD failed to load OpenGL functions.");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer back-ends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;

    // Preview Essentials ========================================================================
    
    GLuint vbo = 0;
    GLuint ebo = 0;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);

    GLfloat quad_vertices[20]
    {
         -1,  -1, 0,  0, 1, // top right
         1, -1, 0,  1, 1, // bottom right
        1, 1, 0,  1, 0, // bottom left
        -1,  1, 0,  0, 0  // top left 
    };

    GLuint quad_index[]{
        0,1,2,
        2,3,0
    };
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(GLfloat), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void**)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void**)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), quad_index, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    constexpr char vertex_shader_source[] = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
    )";
    const char* const vertex_shader_source_ptr = vertex_shader_source;
    auto vertex_shader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source_ptr, nullptr);
    glCompileShader(vertex_shader);
    int  success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }else
    {
        std::cout << "succeed to compile vertex shader\n";
    }


    constexpr char fragment_shader_source[] = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);
}
    )";
    const char* const fragment_shader_source_ptr = fragment_shader_source;
    auto fragment_shader=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source_ptr, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }else
    {
        std::cout << "succeed to compile fragment shader\n";
    }
        
    auto shader_program=glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
    }else
    {
        std::cout << "succeed to link the shader program\n";
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    

    // Render parameters =====================================================================
    gui_tools::shoot_params shoot_photo_params;

    std::vector<gui_tools::object_imported> objects_vector;

    constexpr int filename_max_length = 101;
    char file_save_name[filename_max_length] = "new image";

    std::vector<glm::dvec3> preview_image;

    //  =======================================================================================

    gui_tools::object_imported temp_light_object;
    


    const std::filesystem::path model_path = std::filesystem::current_path() / "scenes";
    std::cout << "Display the obj files in path: " << model_path.string() << std::endl;
    const std::vector<std::filesystem::path> obj_found_in_path = gui_tools::get_models_in_folder(model_path);
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


        // 2. Show render control window
        {
            ImGui::Begin("Path Tracing");

            // Render params
            gui_render_widgets::show_render_params(shoot_photo_params);

            // Display all imported objects
            gui_render_widgets::show_imported_objects(objects_vector);

            // Press button to add a new object 
            gui_render_widgets::show_add_object(obj_found_in_path, objects_vector);
            
            ImGui::InputText("file name", file_save_name, IM_ARRAYSIZE(file_save_name));
            ImGui::InputInt("Side samples per pixel", &shoot_photo_params.side_spp);

            // Output gui_params as json
            if (ImGui::Button("\nOutput json"))
            {
                std::string save_name{ file_save_name };
                nlohmann::json test_json = generate_render_params(shoot_photo_params, save_name, shoot_photo_params.camera_eye_pos, shoot_photo_params.camera_look_at, shoot_photo_params.output_image_size, objects_vector);
                std::ofstream out(save_name + ".json");
                out << test_json;
            }
            ImGui::SameLine();

            // Preview
            if (ImGui::Button("Preview"))
            {
                std::unique_ptr<Camera> camera_for_preview;
                std::string preview_save_name{ file_save_name };
                nlohmann::json preview_render_json = generate_render_params(shoot_photo_params, preview_save_name, shoot_photo_params.camera_eye_pos, shoot_photo_params.camera_look_at, shoot_photo_params.output_image_size, objects_vector);
                try
                {
                    camera_for_preview = std::make_unique<Camera>(preview_render_json, shoot_photo_params.is_photon_map);
                }
                catch (const std::exception& ex)
                {
                    std::cout << ex.what() << std::endl;
                    return -1;
                }

                preview_image = camera_for_preview->preview();

                std::cerr << "Copy complete\n";
                
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                std::vector<glm::vec3> texture_vec;
                texture_vec.reserve(preview_image.size());
                for (int i = 0; i < preview_image.size(); i++)
                {
                    texture_vec.emplace_back(preview_image[i]);
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, shoot_photo_params.preview_image_size.at(0), shoot_photo_params.preview_image_size.at(1), 0, GL_RGB, GL_FLOAT, texture_vec.data());
                glUseProgram(shader_program);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);
                glUseProgram(0);
                std::cout << "done\n";
            }

            if (ImGui::Button("\nStart offline Rendering"))
            {
                std::unique_ptr<Camera> camera;
                std::string save_name{ file_save_name };
                nlohmann::json render_json = generate_render_params(shoot_photo_params, save_name, shoot_photo_params.camera_eye_pos, shoot_photo_params.camera_look_at, shoot_photo_params.output_image_size, objects_vector);

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

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
        while (GLenum error = glGetError()) {
            std::cerr << "openGL error code: " << error << "\n";
        }
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteBuffers(1,&vbo);
    glDeleteBuffers(1,&ebo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int test_render_main()
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

    return 0;
}
