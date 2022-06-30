#pragma once

#include "LOG.h"
#include "GLProgram.h"

class Application
{
private:
    struct MouseData
    {
        int nPosX, nPosY;
        int nImagePosX, nImagePosY;

        // Normal (not flipped) position of a mouse on image
        int nImageNormalPosX, nImageNormalPosY;

        int nLastImagePosX, nLastImagePosY;

        bool bLeftMouseButtonPressed = false;
        bool bShiftPressed, bCtrlPressed;
    };

    struct BrushData
    {
        int nSize = 5;
        int nMinSize = 1, nMaxSize = 300;
        float fColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    struct ImageData
    {
        GLuint RenderID;
        int nWidth, nHeight, nBPP;
        unsigned char* Buffer = nullptr;

        int nZoom = 100;
        int nTranslateX = 0, nTranslateY = 0;
    };

private:
    // Pointer to a main window
    GLFWwindow* m_Window;
    int m_nWindowWidth, m_nWindowHeight; // window size data

    // vertex buffer ID
    GLuint m_VertexBufID;

    GLProgram m_Shader;

    MouseData m_MouseData; // Mouse data
    ImageData m_ImageData; // Image data
    BrushData m_BrushData; // Brush data

private:
    void FreeImageMemory();

    void SetPixel(int x, int y, unsigned char pixel[4]);

public:
    Application(GLFWwindow* window);
    ~Application();

    void LoadImageFile(const char* path);
    void ExportImage(const char* path);
    void RegenerateImageTexture();

    // Initializes app
    void Init();

    // Renders all the UI
    void RenderUI();

    // Processes drawing logic
    void ProcessDrawing();

    // Renders image
    void RenderImage();

    // == Callbacks wrappers ==
    
    void OnScroll(int x, int y);
    void OnMouseMove(int x, int y);
    void OnMouseButton(int button, int action, int mods);
    void OnKeyAction(int key, int scancode, int action, int mods);

public: // Callbacks
    static void ScrollCallback(GLFWwindow* window, double offsetX, double offsetY);
    static void CursorPosCallback(GLFWwindow* window, double posX, double posY);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

};
