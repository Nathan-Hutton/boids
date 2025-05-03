#include "Boid.h"
#include "Camera.h"
#include <vector>

std::vector<Boid> Boid::boids{};

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;

    // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
    const float triangleWidth{ Camera::width / 110.0f };
    const float triangleHeight{ Camera::height / 80.0f };
    const GLfloat vertices[]
    {
        -triangleWidth / 2.0f, triangleHeight,   
        triangleWidth / 2.0f, triangleHeight,    
        0.0f, -triangleHeight                      
    };

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Boid::render() const
{
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
