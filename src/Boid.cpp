#include "Boid.h"
#include <vector>
#include <iostream>

std::vector<Boid> Boid::boids{};
float Boid::triangleWidth{};
float Boid::triangleHeight{};
float Boid::radius{};
float Boid::visionAngle{};

namespace
{
    glm::vec2 rotate(const glm::vec2& v, float angle)
    {
        const float c{ glm::cos(angle) };
        const float s{ glm::sin(angle) };
        return glm::vec2
        {
            v.x * c - v.y * s,
            v.x * s + v.y * c
        };
    }
}

void Boid::init(float screenWidth, float screenHeight)
{
    triangleWidth = screenWidth / 220.0f;
    triangleHeight = screenHeight / 80.0f;
    radius = screenWidth / 25.0f;
    visionAngle = glm::radians(310.0f);
}

void Boid::updateBoids(float deltaTime)
{
    for (size_t i{ 0 }; i < boids.size(); ++i)
    {
        Boid& boid1{ boids[i] };
        for (size_t j{ 1 }; j < boids.size(); ++j)
        {
            if (i == j) continue;
            Boid& boid2{ boids[j] };

            const glm::vec2 vecToOther{ boid2.m_pos - boid1.m_pos };
            if (glm::length(vecToOther) > radius) continue;

            const glm::vec2 dirToOther{ glm::normalize(vecToOther) };
            if (glm::dot(rotate(glm::vec2{ 0.0f, 1.0f}, boid1.m_rotation), dirToOther) < glm::cos(visionAngle / 2.0f)) continue;
        }
    }
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;

    // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
    const GLfloat vertices[]
    {
        -triangleWidth, -triangleHeight,   
        triangleWidth, -triangleHeight,    
        0.0f, triangleHeight                      
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
