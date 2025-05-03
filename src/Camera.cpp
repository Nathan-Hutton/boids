#include "Camera.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
    glm::mat4 viewProjection{ 1.0f };
    glm::vec2 cameraCenter{ 0.0f };
    float width{};
    float height{};
    
    void init(int i_width, int i_height)
    {
        width = static_cast<float>(i_width);
        height = static_cast<float>(i_height);
        cameraCenter = glm::vec2{ width / 2.0f, height / 2.0f };

        const glm::mat4 view{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }) };
        const glm::mat4 projection{ glm::ortho(0.0f, width, 
                                                height, 0.0f, 
                                                -1.0f, 1.0f) };
        viewProjection = projection * view;

    }
}
