#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
    const glm::mat4 view{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f }) };
    const glm::mat4 projection{ glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f) };
    const glm::mat4 viewProjection{ projection * view };
}
