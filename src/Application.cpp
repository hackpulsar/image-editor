#include "GL\glew.h"
#include "Application.h"

#include "Settings.h"
#include "Math.h"

#include "nfd.h"
#include "imgui/imgui.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "glm/gtc/matrix_transform.hpp"

#include "Shaders.h"

#include <string>
#include <Windows.h>

static float g_vertex_buffer_data[] = {
    0.5f,  0.5f,  0.0f,  1.0f, 1.0f,  0.0f,
    0.5f, -0.5f,  0.0f,  1.0f, 0.0f,  0.0f,
    -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f,
    -0.5f, -0.5f,  0.0f,  0.0f, 0.0f,  0.0f,
};

Application::Application(GLFWwindow* window)
    :m_Window(window)
{
    glfwGetWindowSize(m_Window, &m_nWindowWidth, &m_nWindowHeight);
    glfwSetWindowUserPointer(m_Window, static_cast<void*>(this));
}

Application::~Application()
{
    glfwSetWindowUserPointer(m_Window, nullptr);

    Application::FreeImageMemory();
    m_Shader.~GLProgram();
    glDeleteBuffers(1, &m_VertexBufID);
}

void Application::LoadImageFile(const char* path)
{
    this->FreeImageMemory();

    stbi_set_flip_vertically_on_load(1);
    m_ImageData.Buffer = stbi_load(path, &m_ImageData.nWidth, &m_ImageData.nHeight, &m_ImageData.nBPP, 4);

    // Reset zoom
    m_ImageData.nZoom = 100.0f;

    this->RegenerateImageTexture();
}

void Application::ExportImage(const char* path)
{
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(path, m_ImageData.nWidth, m_ImageData.nHeight, m_ImageData.nBPP, m_ImageData.Buffer, m_ImageData.nWidth * m_ImageData.nBPP);
}

void Application::RegenerateImageTexture()
{
    glDeleteTextures(1, &m_ImageData.RenderID);

    glGenTextures(1, &m_ImageData.RenderID);
    glBindTexture(GL_TEXTURE_2D, m_ImageData.RenderID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_ImageData.nWidth, m_ImageData.nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageData.Buffer);
}

void Application::Init()
{
    // Setup vertex array and sh*t
    glGenBuffers(1, &m_VertexBufID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

    // Init shader
    m_Shader.Init(vertexShaderSrc, fragmentShaderSrc);
    m_Shader.Bind();

    //this->LoadImage("res\\bodoamat.png");
}

void Application::RenderUI()
{
    static bool open_file = false;
    static bool bExportingPNG = false;
    static bool bExportingJPG = false;

    if (open_file)
    {
        nfdchar_t* outPath = NULL;
        nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

        if (result == NFD_OKAY) {
            this->LoadImageFile(outPath);
        }
        else if (result == NFD_CANCEL) {
            puts("User pressed cancel.");
        }
        else {
            printf("Error: %s\n", NFD_GetError());
        }

        open_file = false;
    }

    if (bExportingPNG || bExportingJPG)
    {
        if (m_ImageData.Buffer != nullptr)
        {
            nfdchar_t* outPath = NULL;
            nfdresult_t result = NFD_SaveDialog(bExportingJPG ? "jpg" : "png", bExportingJPG ? "out.jpg" : "out.png", &outPath);

            std::string fullPath = std::string(outPath) + (bExportingJPG ? ".jpg" : ".png");

            if (result == NFD_OKAY) {
                this->ExportImage(fullPath.c_str());
            }
            else if (result == NFD_CANCEL) {
                puts("User pressed cancel.");
            }
            else {
                printf("Error: %s\n", NFD_GetError());
            }
        }
        else
        {
            MessageBox(NULL, L"No image to export.", L"Export error", MB_OK | MB_ICONEXCLAMATION);
        }

        bExportingPNG = false;
        bExportingJPG = false;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiWindowFlags host_window_flags = 0;
    host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
    host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        host_window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Window", nullptr, host_window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, nullptr);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Open", NULL, &open_file);
            if (ImGui::BeginMenu("Export as..."))
            {
                ImGui::MenuItem("PNG", NULL, &bExportingPNG);
                ImGui::MenuItem("JPG", NULL, &bExportingJPG);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();

#ifndef _DEBUG
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
#endif
    ImGui::Begin("Instruments");
    ImGui::ColorEdit4("Brush color", m_BrushData.fColor);
    ImGui::DragInt("Brush size", &m_BrushData.nSize, 1.0F, m_BrushData.nMinSize, m_BrushData.nMaxSize);
    ImGui::End();

#ifdef _DEBUG
    ImGui::Begin("Debug data");

    std::string globalMousePosText = "Mouse position global: " + std::to_string(m_MouseData.nPosX) + " : " + std::to_string(m_MouseData.nPosY);
    std::string imageMousePosText = "Mouse position on image: " + std::to_string(m_MouseData.nImageNormalPosX) + " : " + std::to_string(m_MouseData.nImageNormalPosY);
    std::string isPaintingText = "Is painting: " + std::to_string(m_MouseData.bLeftMouseButtonPressed);
    std::string imageSizeText = "Width: " + std::to_string(m_ImageData.nWidth) + " ; Height: " + std::to_string(m_ImageData.nHeight);

    ImGui::Text(globalMousePosText.c_str());
    ImGui::Text(imageMousePosText.c_str());
    ImGui::Text(isPaintingText.c_str());
    ImGui::Text(imageSizeText.c_str());

    ImGui::End();
#endif // DEBUG
}

void Application::ProcessDrawing()
{
    // Draw logic
    if (m_MouseData.bLeftMouseButtonPressed)
    {
        // Check if mouse is on image
        if (m_MouseData.nImagePosX < 0 || m_MouseData.nImagePosX > m_ImageData.nWidth
            || m_MouseData.nImagePosY < 0 || m_MouseData.nImagePosY > m_ImageData.nHeight)
        {
            return;
        }

        unsigned char pixel[4];
        for (int b = 0; b < m_ImageData.nBPP; b++)
        {
            pixel[b] = static_cast<unsigned char>(m_BrushData.fColor[b] * 255.0f);
        }

        glm::vec2 from = glm::vec2(m_MouseData.nLastImagePosX, m_MouseData.nLastImagePosY);
        glm::vec2 to = glm::vec2(m_MouseData.nImagePosX, m_MouseData.nImagePosY);

        auto dir = to - from;
        auto ndir = glm::normalize(dir);

        glm::vec2 pixelPos = from;

        for (int i = 0; i < glm::length(dir) / glm::length(ndir); i++)
        {
            // Draw in brush size
            for (int i = 0; i < m_BrushData.nSize; i++)
            {
                for (int j = 0; j < m_BrushData.nSize; j++)
                {
                    if (int(pixelPos.x + i) >= m_ImageData.nWidth ||
                        int(pixelPos.y + j) >= m_ImageData.nHeight || 
                        int(pixelPos.x + i) <= 0 || int(pixelPos.y + j) <= 0)
                    {
                        continue;
                    }

                    this->SetPixel(int(pixelPos.x + i), int(pixelPos.y + j), pixel);
                }
            }

            //this->SetPixel(int(pixelPos.x), int(pixelPos.y), pixel);
            pixelPos += ndir;
        }

        this->RegenerateImageTexture();
    }
}

void Application::RenderImage()
{
    // Load texture
    if (m_ImageData.Buffer != nullptr) // if some image is loaded
    {
        this->ProcessDrawing();

        m_Shader.Bind();
        
        auto zoom = glm::scale(glm::mat4(1.0f), glm::vec3(float(m_ImageData.nZoom) / 100.0f));
        auto translate = glm::translate(zoom, glm::vec3(m_ImageData.nTranslateX, -m_ImageData.nTranslateY, 0.0f));
        auto scale = glm::scale(translate, glm::vec3(m_ImageData.nWidth, m_ImageData.nHeight, 1.0f));

        glm::mat4 proj = glm::ortho<float>(-(WINDOW_WIDTH / 2.0f), WINDOW_WIDTH / 2.0f, -(WINDOW_HEIGHT / 2.0f), WINDOW_HEIGHT / 2.0f, -1.0f, 1.0f);
        
        m_Shader.SetUniformMat4f("u_MVP", proj * scale);

        glBindTexture(GL_TEXTURE_2D, m_ImageData.RenderID);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Application::OnScroll(int x, int y)
{
    if (m_MouseData.bShiftPressed)
    {
        m_ImageData.nTranslateX += (y * 5);
    }
    else if (m_MouseData.bCtrlPressed)
    {
        m_ImageData.nTranslateY += (y * 5);
    }
    else
    {
        m_ImageData.nZoom += (y * 5);
        m_ImageData.nZoom = Math::clamp<int>(m_ImageData.nZoom, 1.0f, 500.0f);
    }
}

void Application::OnMouseMove(int x, int y)
{
    // Set global mouse position
    m_MouseData.nPosX = x;
    m_MouseData.nPosY = y;

    // Mouse position on image
    if (m_ImageData.Buffer != nullptr) // if image is loaded
    {
        auto zoom = glm::scale(glm::mat4(1.0f), glm::vec3(float(m_ImageData.nZoom) / 100.0f));
        auto translate = glm::translate(zoom, glm::vec3(m_ImageData.nTranslateX, m_ImageData.nTranslateY, 0.0f));
        auto scale = glm::scale(translate, glm::vec3(m_ImageData.nWidth, m_ImageData.nHeight, 1.0f));

        glm::mat4 proj = glm::ortho<float>(-(WINDOW_WIDTH / 2.0f), WINDOW_WIDTH / 2.0f, -(WINDOW_HEIGHT / 2.0f), WINDOW_HEIGHT / 2.0f, -1.0f, 1.0f);

        auto pp = glm::unProject(glm::vec3(x, y, 0.0f),
            translate, proj,
            glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));

        m_MouseData.nLastImagePosX = m_MouseData.nImagePosX;
        m_MouseData.nLastImagePosY = m_MouseData.nImagePosY;

        m_MouseData.nImagePosX = int(pp.x) + (m_ImageData.nWidth / 2);
        m_MouseData.nImagePosY = -(int(pp.y) - (m_ImageData.nHeight / 2));

        m_MouseData.nImageNormalPosX = int(pp.x) + (m_ImageData.nWidth / 2);
        m_MouseData.nImageNormalPosY = int(pp.y) + (m_ImageData.nHeight / 2);

        // clamp mouse image pos
        //m_MouseData.nImagePosX = Math::clamp<int>(m_MouseData.nImagePosX, 0, m_ImageData.nWidth);
        //m_MouseData.nImagePosY = Math::clamp<int>(m_MouseData.nImagePosY, 0, m_ImageData.nHeight);

        m_MouseData.nImageNormalPosX = Math::clamp<int>(m_MouseData.nImageNormalPosX, 0, m_ImageData.nWidth);
        m_MouseData.nImageNormalPosY = Math::clamp<int>(m_MouseData.nImageNormalPosY, 0, m_ImageData.nHeight);
    }
}

void Application::OnMouseButton(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        m_MouseData.bLeftMouseButtonPressed = true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        m_MouseData.bLeftMouseButtonPressed = false;
}

void Application::OnKeyAction(int key, int scancode, int action, int mods)
{
    m_MouseData.bShiftPressed = (mods & GLFW_MOD_SHIFT);
    m_MouseData.bCtrlPressed = (mods & GLFW_MOD_CONTROL);
}

void Application::ScrollCallback(GLFWwindow* window, double offsetX, double offsetY)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    
    if (app != nullptr) app->OnScroll(int(offsetX), int(offsetY));
}

void Application::CursorPosCallback(GLFWwindow* window, double posX, double posY)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->OnMouseMove(int(posX), int(posY));
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->OnMouseButton(button, action, mods);
}

void Application::KeyCallback(GLFWwindow* window, int button, int key, int action, int mods)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->OnKeyAction(button, key, action, mods);
}

void Application::FreeImageMemory()
{
    glDeleteTextures(1, &m_ImageData.RenderID);
    stbi_image_free(m_ImageData.Buffer);
    m_ImageData.Buffer = nullptr;
}

void Application::SetPixel(int x, int y, unsigned char pixel[4])
{
    if (m_ImageData.Buffer == nullptr) return;

    int position = y * m_ImageData.nWidth + x;

    for (int b = 0; b < 4; b++)
    {
        m_ImageData.Buffer[position * 4 + b] = pixel[b];
    }
}
