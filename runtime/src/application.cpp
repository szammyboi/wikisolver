#include "application.h"
#include "wikipedia.h"
#include <stdexcept>
#include <iostream>
#include <chrono>

// Constructor for unsized app
Application::Application()
{
    InitWindow();
    InitOpenGL();
    InitImGUI();
}

// Constructor for sized app
Application::Application(unsigned width, unsigned height)
    : m_Width(width), m_Height(height)
{
    InitWindow();
    InitOpenGL();
    InitImGUI();
}

void Application::Resize(unsigned width, unsigned height)
{
    m_Width = width;
    m_Height = height;
}

// GLFW calls this function when the window resizes
void resize_callback(GLFWwindow* window, int width, int height)
{
    // Set the apps size to the new window size
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    glViewport(0, 0, width, height);
    app->Resize(width, height);
}

// Initalizes the GLFW Window
void Application::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    m_Window = glfwCreateWindow(m_Width, m_Height, "Wikisolver", NULL, NULL);
    if (m_Window == NULL)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_Window);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, resize_callback);
}

// Load all the opengl functions
void Application::InitOpenGL()
{
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
        throw std::runtime_error("Failed to initialize OpenGL context");
}

// Initalize IMGUI
void Application::InitImGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    const char* glsl_version = "#version 460";
    ImGui::StyleColorsDark();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls 

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

// Adds a font to the font map
void Application::AddFont(const std::string& name, const std::string& path, int size)
{
    // Create an imgui font and insert it into the fonts map
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size);
    m_Fonts[name] = font;
}

// Retrieves a font
ImFont* Application::GetFont(std::string name)
{
    return m_Fonts[name];
}


// Used to manage the state of the search box dropdown
struct PopupState
{
    int current = 0;
    bool open = false;
    bool selected = false;
    int lastBufferSize = 0;
    std::vector<const Article *> options;
};

// Callback when a search box is interacted with
int AutoCompleteCallback(ImGuiInputTextCallbackData* data)
{
    PopupState& popup_state = *reinterpret_cast<PopupState*>( data->UserData );

    // If the data buffer changed size (a character was entered)
    // Open the popup, and retrieve the new search options
    if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
    {
        if (popup_state.lastBufferSize != data->BufTextLen)
        {
            popup_state.options = WikipediaSolver::SearchTitle(data->Buf, 5);
            popup_state.open = true;
            popup_state.lastBufferSize = data->BufTextLen;
        }
    }

    // If the up or down arrow was pressed
    // Adjust the current option being chosen
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
    {
        popup_state.open = true;
        if( data->EventKey == ImGuiKey_UpArrow && popup_state.current > 0)
            popup_state.current--;
        if( data->EventKey == ImGuiKey_DownArrow && popup_state.current < popup_state.options.size()-1)
            popup_state.current++;
    }

    return 0;
}

// Search Box Text Input
void TextFilter(std::string name, std::string& input, PopupState& state)
{
    // Create a text input and link it to the appropriate callbacks and flags
    ImGuiInputTextFlags input_flags =  ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory;
    std::string label = "## " + name;

    // If 'enter' was pressed in the textbox, assign the currently selected
    // option to the text box
    if (ImGui::InputText(label.c_str(), &input, input_flags, AutoCompleteCallback, &state))
    {
        if (state.options.size() > state.current)
            input.assign(state.options[state.current]->title);
    }

    // If the text box is unfocused close the dropdown
    if ((ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
         && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked( 0 )) 
         || state.options.size() == 0)
    {
        state.open = false;
        state.current = 0;
    }
}

// Search Box Dropdown
void TextOptions(std::string name, std::string& input, PopupState& state, int width)
{
    // If not open, don't draw anything
    if (!state.open) return;
    
    // For each option, draw a selectable option
    // If the options is chosen (clicked), update the string to match
    std::string childLabel = name + "child";
    ImGui::BeginChild(childLabel.c_str());
    int i = 0;
    for (auto* article : state.options)
    {
        if(ImGui::Selectable(article->title.c_str(), i==state.current, 0, ImVec2(width,0)))
        {
            state.current = i;
            state.open = false;
            if (state.options.size() > state.current)
                input.assign(state.options[state.current]->title);
        }
        i++;
    }
    ImGui::EndChild();
}

void Application::Run()
{
    // Load necessary data and assets
    WikipediaSolver::LoadData("data_collection/data.bin");
    AddFont("title", "assets/fonts/JetBrains_Mono/static/JetBrainsMono-Bold.ttf", 28);
    AddFont("subtitle", "assets/fonts/JetBrains_Mono/static/JetBrainsMono-Bold.ttf", 24);

    // Values for the from dropdown
    std::string from_input;
    PopupState from_state;

    // Values for the to dropdown
    std::string to_input;
    PopupState to_state;

    // Arrays to hold the algorithm results
    std::vector<const Article*> bfs_result;
    std::vector<const Article*> iddfs_result;

    // Variables to hold the algorithm times
    long long bfs_time = 0;
    long long iddfs_time = 0;

    bool searched = false;

    while (!glfwWindowShouldClose(m_Window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(m_Width, m_Height));
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::Begin("WikiSolver", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Draw the left dropdown
        ImGui::SameLine(0, m_Width/12);
        ImGui::BeginChild("Left", ImVec2(m_Width/3, m_Height/4));
            ImGui::SetNextItemWidth(m_Width/3);
            TextFilter("from input", from_input, from_state);
            TextOptions("from input", from_input, from_state, m_Width/3);
        ImGui::EndChild();

        ImGui::SameLine(0, m_Width/6);

        // Draw the right dropdown
        ImGui::BeginChild("Right", ImVec2(m_Width/3, m_Height/4));
            ImGui::SetNextItemWidth(m_Width/3);
            TextFilter("to input", to_input, to_state);
            TextOptions("to input", to_input, to_state, m_Width/3);
        ImGui::EndChild();

        // Draw the Go! button
        // If clicked run both algorithms and update their results and times
        ImGui::BeginChild("Button", ImVec2(), ImGuiChildFlags_AutoResizeY);
        ImGui::SameLine(0, m_Width/3);
        if (ImGui::Button("Go!", ImVec2(m_Width/3, 0)))
        {
            if (from_state.options.size() > 0 && to_state.options.size() > 0)
            {
                auto start = std::chrono::high_resolution_clock::now();
                bfs_result = WikipediaSolver::FindPathBFS(from_input, to_input);
                auto end = std::chrono::high_resolution_clock::now();

                bfs_time = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

                start = std::chrono::high_resolution_clock::now();
                iddfs_result = WikipediaSolver::FindPathIDDFS(from_input, to_input);
                end = std::chrono::high_resolution_clock::now();

                iddfs_time = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
                searched = true;
            }
        }
        ImGui::EndChild();

        auto size = ImGui::GetItemRectSize();

        ImGui::BeginChild("spacer", ImVec2(m_Width, size.y/2)); ImGui::EndChild();
        ImGui::Separator();
        ImGui::BeginChild("spacer2", ImVec2(m_Width, size.y/2)); ImGui::EndChild();

        // Draw the algorithm results
        // For each algorithm,
        // Draw text boxes for each page in the result path
        // or draw "No Path Found!"
        ImGui::BeginChild("Results");
            ImGui::PushFont(GetFont("subtitle"));
            ImGui::SameLine(0, m_Width/12);
            ImGui::BeginChild("BFS Results", ImVec2(m_Width/3, -1));
            ImGui::PushItemWidth(m_Width/3);
            std::string bfs_time_result = "BFS Time: " + std::to_string(bfs_time) + "ms";
            ImGui::Text(bfs_time_result.c_str());
            int i = 1;
            for (auto* article : bfs_result)
            {
                std::string entry = std::to_string(i) + ". " + article->title.c_str();
                ImGui::Text(entry.c_str());
                i++;
            }
            if (bfs_result.size() == 0 && searched)
            {
                ImGui::Text("No Path Found!");
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::SameLine(0, m_Width/6);

            ImGui::BeginChild("IDDFS Results", ImVec2(m_Width/3, -1));
            ImGui::PushItemWidth(m_Width/3);
            std::string iddfs_time_result = "IDDFS Time: " + std::to_string(iddfs_time) + "ms";
            ImGui::Text(iddfs_time_result.c_str());
            i = 1;
            for (auto* article : iddfs_result)
            {
                std::string entry = std::to_string(i) + ". " + article->title.c_str();
                ImGui::Text(entry.c_str());
                i++;
            }
            if (iddfs_result.size() == 0 && searched)
            {
                ImGui::Text("No Path Found!");
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::PopFont();
        ImGui::EndChild();

        ImGui::End();

        ImGui::Render();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}