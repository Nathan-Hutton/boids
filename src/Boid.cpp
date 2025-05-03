#include "Boid.h"
#include <vector>
#include <iostream>

std::vector<Boid> Boid::boids{};
float Boid::triangleWidth{};
float Boid::triangleHeight{};
float Boid::radius{};
float Boid::visionAngle{};

void Boid::init(float screenWidth, float screenHeight)
{
    triangleWidth = screenWidth / 220.0f;
    triangleHeight = screenHeight / 80.0f;
    radius = screenWidth / 25.0f;
    visionAngle = 310.0f;
}

void Boid::updateBoids(float deltaTime)
{
    for (const Boid& boid : Boid::boids)
    {
        std::cout << radius << '\n';
        std::cout << triangleWidth << '\n';
        std::cout << triangleHeight << '\n';
        std::cout << '\n';
    }
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;

    // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
    const GLfloat vertices[]
    {
        -triangleWidth, triangleHeight,   
        triangleWidth, triangleHeight,    
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
