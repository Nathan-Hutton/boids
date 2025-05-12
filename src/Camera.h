#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
    extern glm::mat4 viewProjection;
    extern glm::vec2 cameraCenter;
    extern float screenHeight;
    extern float screenWidth;
    extern GLFWwindow* window;

    void init(GLFWwindow* window, float screenWidth, float screenHeight);
}
