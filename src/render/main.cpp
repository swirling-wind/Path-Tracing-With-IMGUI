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
#include <glad/glad.h>

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

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}


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


//#if defined(IMGUI_IMPL_OPENGL_ES2)
//    // GL ES 2.0 + GLSL 100
//    const char* glsl_version = "#version 100";
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
//#elif defined(__APPLE__)
//    // GL 3.2 + GLSL 150
//    const char* glsl_version = "#version 150";
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
//#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Path Tracing", nullptr, nullptr);
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

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;

    // Preview Params ========================================================================
    
    GLuint vbo = 0;
    GLuint ebo = 0;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);

    GLfloat quad_vertices[20]
    {
        // positions         // texture coords
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
    glShaderSource(vertex_shader, 1, &vertex_shader_source_ptr, NULL);
    glCompileShader(vertex_shader);
    int  success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
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
    glShaderSource(fragment_shader, 1, &fragment_shader_source_ptr, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
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
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
    }else
    {
        std::cout << "succeed to link the shader program\n";
    }


    // import image for test

    //int my_image_width = 0;
    //int my_image_height = 0;
    //GLuint my_image_texture = 0;
    //bool ret = LoadTextureFromFile("test.png", &my_image_texture, &my_image_width, &my_image_height);
    //IM_ASSERT(ret);
    //glUseProgram(shader_program);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, my_image_texture);
    //glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0); // ÊÖ¶¯ÉèÖÃ
    //glUseProgram(0);

    // load and create a texture 
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    
    // load image, create texture and generate mipmaps
    int texture_width, texture_height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load("test.png", &texture_width, &texture_height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Render parameters =====================================================================
    gui_tools::shoot_params shoot_photo_params;

    std::array<int, 2> output_image_size = { 1280, 720 };
    const std::array<int, 2> preview_image_size = { 1280, 720 };

    std::vector<gui_tools::object_imported> objects_vector;

    std::array<float, 3>camera_eye_pos = { -0.2f, 2.2f, 10.0f };
    std::array<float, 3>camera_look_at = { -0.2f, 1.7f, 0.0f };

    constexpr int filename_max_length = 101;
    char file_save_name[filename_max_length] = "new image";

    std::vector<glm::dvec3> preview_image;

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
        {
            //ImGui::Begin("OpenGL Texture Text");
            //ImGui::Text("pointer = %p", my_image_texture);
            //ImGui::Text("size = %d x %d", my_image_width, my_image_height);
            //ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
            //ImGui::End();
            ImGui::Begin("OpenGL Texture Text");
            ImGui::Text("pointer = %p", texture);
            ImGui::Text("size = %d x %d", texture_width, texture_height);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(texture_width, texture_height));
            ImGui::End();
        }
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

            // Render
            if (ImGui::Button("Preview"))
            {
                std::unique_ptr<Camera> camera_for_preview;
                std::string preview_save_name{ file_save_name };
                //int temp_ssq = shoot_photo_params.side_spp;//swap and temperately let ssp as 1
                //shoot_photo_params.side_spp = 2;
                nlohmann::json preview_render_json = generate_render_params(shoot_photo_params, preview_save_name, camera_eye_pos, camera_look_at, output_image_size, objects_vector);
                //shoot_photo_params.side_spp = temp_ssq;
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
                glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // set texture filtering parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                std::vector<float> texture_vec;

                unsigned char* image_data = stbi_load("test.png", &texture_width, &texture_height, NULL, 4);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
                //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
                //stbi_image_free(image_data);
                ;
                texture_vec.reserve(preview_image.size() *4);
                for(int i=0;i< preview_image.size() ;i++)
                {
                    //texture_vec.emplace_back(preview_image[i]);
                    //texture_vec.emplace_back(glm::vec3(1.0,0,0));
                    texture_vec.push_back(preview_image[i].r);
                    texture_vec.push_back(preview_image[i].g);
                    texture_vec.push_back(preview_image[i].b);
                    //texture_vec.push_back(0);
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, preview_image_size.at(0), preview_image_size.at(1), 0, GL_RGB, GL_FLOAT, texture_vec.data());
                glUseProgram(shader_program);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);
                glUseProgram(0);
                texture_width= preview_image_size.at(0);
                texture_height = preview_image_size.at(1);
                std::cout << "done\n";
            }

            if (ImGui::Button("\nStart offline Rendering in Current Setting"))
            {
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

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        //{
        //glBindVertexArray(vao);
        //glBindBuffer(GL_ARRAY_BUFFER, vbo);
        //double image_w = preview_image_size[0];
        //double image_h = preview_image_size[1];
        //if (display_w / display_h > image_w / image_h)
        //{
        //    double ratio = image_w / image_h * display_h / display_w;
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 0)) = glm::vec2(-ratio, -1);
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 5)) = glm::vec2(ratio, -1);
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 10)) = glm::vec2(ratio, 1);
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 15)) = glm::vec2(-ratio, 1);
        //}
        //else
        //{
        //    double ratio = image_h / image_w * display_w / display_h;
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 0)) = glm::vec2(-1, -ratio);
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 5)) = glm::vec2(1, -ratio);
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 10)) = glm::vec2(1, ratio);
        //    *(reinterpret_cast<glm::vec2*>(reinterpret_cast<float*>(quad_vertices) + 15)) = glm::vec2(-1, ratio);
        //}
        //glBufferData(vbo, 20, quad_vertices, GL_DYNAMIC_DRAW);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //glBindVertexArray(0);
        //}
        // Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        //std::cout << "height: "<<display_h <<"width: "<< display_w<<"\n";
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
