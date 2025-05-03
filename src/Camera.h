#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
    extern glm::mat4 viewProjection;
    extern glm::vec2 cameraCenter;

    void init(float screenWidth, float screenHeight);
}
