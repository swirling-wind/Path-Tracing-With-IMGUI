#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include "../render/build_scene.h"
#include "../render/scene.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <thread>
#include <GLFW/glfw3.h> 
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int test(int, char**)
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
    bool show_another_window = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Path Tracing");                          // Create a window called "Hello, world!" and append into it.

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
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

int main()
{
    const std::map<std::string, std::string> test_params{
    {"samples", "2"},      {"super_samples", "10"},
    {"plane_width", "1.5"},
    {"width_res", "1200"}, {"height_res", "900"},
    {"scene_num", "3"},     {"tracer", "pt"} };

    Scene scene;
    switch (std::stoi(test_params.at("scene_num")))
    {
    case 1:
    {
        if (!instant_renderer::build_1(scene)) return 1;

        Vec eye(50, 50, 220), lookat(50.0, 30.0, -1.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    case 2:
    {
        if (!instant_renderer::build_2(scene)) return 1;

        Vec eye(10, 5, 9), lookat(0.0, 2.0, 0.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    case 3:
    {
        if (!instant_renderer::build_3(scene)) return 1;

        Vec eye(0.0, 2., 5), lookat(0.0, 1.0, 0.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    case 4:
    {
        if (!instant_renderer::build_4(scene)) return 1;

        Vec eye(-1.8, 1.2, 1.8), lookat(0.0, 0.8, 0.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    case 5:
    {
        if (!instant_renderer::build_5(scene)) return 1;

        Vec eye(200.0, 100.0, 200.0), lookat(50.0, 5.0, 50.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    case 6:
    {
        if (!instant_renderer::build_6(scene)) return 1;

        Vec eye(400.0, 200.0, 400.0), lookat(50.0, 5.0, 50.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    default:
    {
        if (!instant_renderer::build_1(scene)) return 1;

        Vec eye(50, 50, 220), lookat(50.0, 30.0, -1.0);
        Camera* pinhole_ptr(new Pinhole(eye, lookat, 1.0));
        scene.set_camera(pinhole_ptr);
        break;
    }
    }

    ViewPlane view_plane{ 1.5, 1200, 900 };

    scene.render(10, view_plane);

    //std::thread render_thread{&Scene::render, scene, test_params};

    return 0;
}
