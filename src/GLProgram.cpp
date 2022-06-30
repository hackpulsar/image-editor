#include "GL\glew.h"

#include "LOG.h"
#include "GLProgram.h"

#include <vector>

GLuint LoadShaderProgram(const char* vertShaderSrc, const char* fragShaderSrc)
{
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    GLint result = GL_FALSE;
    GLint logLength;

    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // Check vertex shader
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        LOG("compiling vertex shader");
        glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> vertShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
        glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
        LOG(&vertShaderError[0]);
    }

    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        LOG("compiling fragment shader");
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> fragShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
        glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
        LOG(&fragShaderError[0]);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        LOG("linking program");
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> programError(static_cast<size_t>((logLength > 1) ? logLength : 1));
        glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
        LOG(&programError[0]);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

int GLProgram::GetUniformLocation(const std::string& name)
{
    int location = glGetUniformLocation(m_Index, name.c_str());
    if (location == -1)
        LOG("[WARNING]: uniform " << name << " doesn`t exist");
    return location;
}

GLProgram::GLProgram() { }
GLProgram::~GLProgram() { glDeleteProgram(m_Index); }

void GLProgram::Init(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc)
{
    m_Index = LoadShaderProgram(vertexShaderSrc.c_str(), fragmentShaderSrc.c_str());
    glUniform1i(this->GetUniformLocation("u_Texture"), 0);
}

void GLProgram::Bind()
{
    glUseProgram(m_Index);
}

void GLProgram::Unbind()
{
    glUseProgram(0);
}

void GLProgram::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(this->GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}
