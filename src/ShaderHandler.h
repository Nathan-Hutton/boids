#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

namespace  ShaderHandler
{
    extern GLuint shaderProgram;
    GLuint compileShader(const std::vector<std::string>& shaderPaths);
}

