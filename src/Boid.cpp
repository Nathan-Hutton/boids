#include "Boid.h"
#include "Camera.h"

#include <random>
#include <vector>
#include <iostream>

std::vector<Boid> Boid::s_boids{};
float Boid::s_maxSpeed{};
float Boid::s_triangleWidth{};
float Boid::s_triangleHeight{};
float Boid::s_radius{};
float Boid::s_visionAngleCos{};

void Boid::init(float screenWidth, float screenHeight)
{
    s_triangleWidth = screenWidth / 220.0f;
    s_triangleHeight = screenHeight / 80.0f;
    s_radius = screenWidth / 25.0f;
    s_visionAngleCos = glm::cos(glm::radians(270.0f) / 2.0f);
    s_maxSpeed = s_radius * 4.0f;
}

void Boid::updateBoids(float deltaTime)
{
    std::vector<glm::vec2> updatedVelocities(s_boids.size());

    // First, we filter out the neighboring boids that aren't actually neighbors (must be within radius and vision angle/cone)
    for (size_t i{ 0 }; i < s_boids.size(); ++i)
    {
        Boid& primaryBoid{ s_boids[i] };
        glm::vec2 updatedVelocity{ primaryBoid.m_velocity };

        int numVisibleBoids{ 0 }; // Including the primary boid itself
        glm::vec2 avgNeighborVelocity{ 0.0f };

        for (size_t j{ 0 }; j < s_boids.size(); ++j)
        {
            if (i == j) continue;
            const Boid& otherBoid{ s_boids[j] };

            glm::vec2 vecToOther{ otherBoid.m_pos - primaryBoid.m_pos };
            if (std::abs(vecToOther.x) > Camera::screenWidth / 2.0f)
                vecToOther.x -= glm::sign(vecToOther.x) * Camera::screenWidth;
            if (std::abs(vecToOther.y) > Camera::screenHeight / 2.0f)
                vecToOther.y -= glm::sign(vecToOther.y) * Camera::screenHeight;

            const float dist{ glm::length(vecToOther) };
            if (dist > s_radius || dist < 1e-6) continue;

            const glm::vec2 dirToOther{ glm::normalize(vecToOther) };
            if (glm::dot(glm::normalize(primaryBoid.m_velocity), dirToOther) < s_visionAngleCos) continue;

            ++numVisibleBoids;
            avgNeighborVelocity += otherBoid.m_velocity;
        }

        if (numVisibleBoids > 0)
        {
            avgNeighborVelocity /= numVisibleBoids;
            const glm::vec2 steeringForce{ avgNeighborVelocity - primaryBoid.m_velocity };
            updatedVelocity = glm::normalize(primaryBoid.m_velocity + steeringForce) * s_maxSpeed;
        }

        updatedVelocities[i] = updatedVelocity;
    }

    for (size_t i{ 0 }; i < s_boids.size(); ++i)
    {
        Boid& boid{ s_boids[i] };

        boid.m_velocity = updatedVelocities[i];
        boid.m_pos += boid.m_velocity * deltaTime;
        if (boid.m_pos.y - s_triangleHeight > Camera::screenHeight)
            boid.m_pos.y -= Camera::screenHeight;
        if (boid.m_pos.y + s_triangleHeight < 0)
            boid.m_pos.y += Camera::screenHeight;
        if (boid.m_pos.x - s_triangleHeight > Camera::screenWidth)
            boid.m_pos.x -= Camera::screenWidth;
        if (boid.m_pos.x + s_triangleHeight < 0)
            boid.m_pos.x += Camera::screenWidth;
    }
}

void Boid::createBoid(glm::vec2 pos)
{
    s_boids.emplace_back(pos);
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;
    {
        // Get a random velocity direction
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist{-1.0f, 1.0f}; // I'm using radius since it's already scaled to the screen size
        m_velocity = glm::normalize(glm::vec2{ dist(gen), dist(gen) });

        // Set a random speed for the velocity
        dist = std::uniform_real_distribution<float>{s_radius / 2.0f, s_maxSpeed};
        m_velocity = m_velocity * dist(gen);
    }

    // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
    const GLfloat vertices[]
    {
        -s_triangleWidth, -s_triangleHeight,   
        s_triangleWidth, -s_triangleHeight,    
        0.0f, s_triangleHeight                      
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
