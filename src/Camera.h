#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
    extern glm::mat4 viewProjection;
    extern glm::vec3 cameraCenter;
    extern float width;
    extern float height;
    extern float aspectRatio;

    void init(int width, int height);
}
