#include "wikipedia.h"

#include <iostream>
#include <vector>
#include <string>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Wikipedia Solver", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    } 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    const char* glsl_version = "#version 460";
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls      // IF using Docking Branch
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool t = false;

    WikipediaSolver::LoadData("data_collection/data.bin");
    
    std::vector<std::string> p = WikipediaSolver::FindPath("scooby doo", "icarly");

    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Results");
            ImGui::BeginChild("Scrolling");
            for (auto& title : p)
                ImGui::Text(title.c_str());
            if (p.empty())
                ImGui::Text("No path!");

            // will pause frame (bad)
            if (ImGui::Button("Randomize"))
                p = WikipediaSolver::RandomRun();
            ImGui::EndChild();
            ImGui::End();
        }
        
        ImGui::Render();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}