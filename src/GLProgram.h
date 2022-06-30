#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

GLuint LoadShaderProgram(const char* vertShaderSrc, const char* fragShaderSrc);

class GLProgram
{
private:
    GLuint m_Index;

private:
    int GetUniformLocation(const std::string& name);


public:
    GLProgram();
    ~GLProgram();

    void Init(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc);

    void Bind();
    void Unbind();

    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
};
