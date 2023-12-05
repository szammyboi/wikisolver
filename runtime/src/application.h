#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <map>
#include <vector>
#include <string>

struct Article;

class Application
{
public: 
    Application();
    Application(unsigned width, unsigned height);
    ~Application() = default;

    void Run();
    void Resize(unsigned width, unsigned height);

    void AddFont(const std::string& name, const std::string& path, int size);
    ImFont* GetFont(std::string name);
private:
    void InitWindow();
    void InitOpenGL();
    void InitImGUI();
private:
    GLFWwindow* m_Window = nullptr;
    unsigned m_Width = 800;
    unsigned m_Height = 600;

    std::map<std::string, ImFont*> m_Fonts;
};
