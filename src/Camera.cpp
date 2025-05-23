#include "Camera.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
    glm::mat4 viewProjection{ 1.0f };
    glm::vec2 cameraCenter{ 0.0f };
    float screenWidth{ 0.0f };
    float screenHeight{ 0.0f };
    GLFWwindow* window{};
    
    void init(GLFWwindow* window, float screenWidth, float screenHeight)
    {
        Camera::window = window;
        Camera::screenWidth = screenWidth;
        Camera::screenHeight = screenHeight;
        cameraCenter = glm::vec2{ screenWidth / 2.0f, screenHeight / 2.0f };

        const glm::mat4 view{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }) };
        const glm::mat4 projection{ glm::ortho(0.0f, screenWidth, 
                                                screenHeight, 0.0f, 
                                                -1.0f, 1.0f) };
        viewProjection = projection * view;

    }
}
