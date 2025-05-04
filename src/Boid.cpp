#include "Boid.h"

#include <random>
#include <vector>
#include <iostream>

std::vector<Boid> Boid::boids{};
float Boid::triangleWidth{};
float Boid::triangleHeight{};
float Boid::radius{};
float Boid::visionAngleCos{};

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
    visionAngleCos = glm::cos(glm::radians(270.0f) / 2.0f);
}

void Boid::updateBoids(float deltaTime)
{
    // First, we filter out the neighboring boids that aren't actually neighbors (must be within radius and vision angle/cone)
    for (size_t i{ 0 }; i < boids.size(); ++i)
    {
        Boid& boid{ boids[i] };
        //const glm::vec2 heading{ rotate(glm::vec2{ 0.0f, 1.0f }, boid.m_rotation) };

        for (size_t j{ 0 }; j < boids.size(); ++j)
        {
            if (i == j) continue;
            const Boid& otherBoid{ boids[j] };

            const glm::vec2 vecToOther{ otherBoid.m_pos - boid.m_pos };
            if (glm::length(vecToOther) > radius || glm::length(vecToOther) < 1e-6) continue;

            const glm::vec2 dirToOther{ glm::normalize(vecToOther) };
            if (glm::dot(glm::normalize(boid.m_velocity), dirToOther) < visionAngleCos) continue;
        }

        boid.m_pos += boid.m_velocity * deltaTime;
    }
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist{-radius * 2.0f, radius * 2.0f}; // I'm using radius since it's already scaled to the screen size
        m_velocity = glm::vec2{ dist(gen), dist(gen) };
    }

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
