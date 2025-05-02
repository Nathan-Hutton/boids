#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>

namespace  ShaderHandler
{
    GLuint compileShader(const std::vector<std::string>& shaderPaths);
}

