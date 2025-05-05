#include "Boid.h"
#include "Camera.h"

#include "imgui.h"

#include <random>
#include <vector>
#include <iostream>

std::vector<Boid> Boid::s_boids{};
GLuint Boid::s_VAO{};
float Boid::s_triangleWidth{};
float Boid::s_triangleHeight{};

float Boid::s_maxSpeed{};
float Boid::s_minSpeed{};
float Boid::s_maxSteeringMagnitude{};
float Boid::s_minSteeringMagnitude{};
float Boid::s_alignmentScale{ 5.0f };

float Boid::s_radius{};
float Boid::s_visionAngleCos{};

namespace rd
{
    std::random_device rd;
    std::mt19937 randomNumberGenerator{rd()};
    std::uniform_real_distribution<float> speedDistribution{};
}

namespace imguiScalars
{
    float radiusScale{ 1.0f / 20.0f };
    float visionAngleDegrees{ 270.0f };
    float maxSpeedScale{ 1.0f / 6.0f };
    float minSpeedScale{ 1.0f / 14.0f };
    float maxSteeringMagnitudeScale{ 0.4f };
    float minSteeringMagnitudeScale{ 1.0f / 3.5f };
}

void Boid::recomputeStaticParams()
{
    s_radius = Camera::screenWidth * imguiScalars::radiusScale;
    s_visionAngleCos = glm::cos(glm::radians(imguiScalars::visionAngleDegrees) / 2.0f);

    s_maxSpeed = Camera::screenWidth * imguiScalars::maxSpeedScale;
    s_minSpeed = Camera::screenWidth * imguiScalars::minSpeedScale;
    s_maxSteeringMagnitude = Camera::screenWidth * imguiScalars::maxSteeringMagnitudeScale;
    s_minSteeringMagnitude = Camera::screenWidth * imguiScalars::minSteeringMagnitudeScale;

    rd::speedDistribution = std::uniform_real_distribution<float>{-s_maxSpeed / 5.0f, s_maxSpeed / 5.0f};
}

void Boid::init()
{
    s_triangleWidth = Camera::screenWidth / 220.0f;
    s_triangleHeight = Camera::screenHeight / 80.0f;

    recomputeStaticParams();
    // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
    const GLfloat vertices[]
    {
        -s_triangleWidth, -s_triangleHeight,   
        s_triangleWidth, -s_triangleHeight,    
        0.0f, s_triangleHeight                      
    };

    glGenVertexArrays(1, &s_VAO);
    glBindVertexArray(s_VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Boid::showImGuiControls()
{
    bool changed{ false };
    changed |= ImGui::SliderFloat("Radius scale", &imguiScalars::radiusScale, 1.0f / 60.0f, 0.5f);
    changed |= ImGui::SliderFloat("Vision angle (degrees)", &imguiScalars::visionAngleDegrees, 0.0f, 360.0f);
    changed |= ImGui::SliderFloat("Max speed scale", &imguiScalars::maxSpeedScale, imguiScalars::minSpeedScale, 0.8f );
    changed |= ImGui::SliderFloat("Min speed scale", &imguiScalars::minSpeedScale, 1.0f / 50.0f, imguiScalars::maxSpeedScale );
    changed |= ImGui::SliderFloat("Max steering force scale", &imguiScalars::maxSteeringMagnitudeScale, imguiScalars::minSteeringMagnitudeScale, 2.0f );
    changed |= ImGui::SliderFloat("Min steering force scale", &imguiScalars::minSteeringMagnitudeScale, 0.05f, imguiScalars::maxSteeringMagnitudeScale );

    if (changed)
        recomputeStaticParams();
}

void Boid::updateBoids(float deltaTime)
{
    std::vector<glm::vec2> updatedVelocities(s_boids.size());

    // First, we filter out the neighboring boids that aren't actually neighbors (must be within radius and vision angle/cone)
    for (size_t i{ 0 }; i < s_boids.size(); ++i)
    {
        Boid& primaryBoid{ s_boids[i] };
        int numVisibleBoids{ 0 };
        glm::vec2 steeringForce{ 0.0f };

        // Alignment
        glm::vec2 avgNeighborVelocity{ 0.0f };
        glm::vec2 updatedVelocity{ primaryBoid.m_velocity };

        // Cohesion


        // Separation

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
            // *********
            // Alignment
            // *********
            avgNeighborVelocity /= numVisibleBoids;

            steeringForce += avgNeighborVelocity - primaryBoid.m_velocity;
            steeringForce *= s_alignmentScale;
        }
        else// Make steering force random
            steeringForce = glm::vec2{ rd::speedDistribution(rd::randomNumberGenerator) / 5.0f, rd::speedDistribution(rd::randomNumberGenerator) / 5.0f };

        // Min/Max clamp for steering force
        const float steeringMagnitude{ glm::length(steeringForce) };
        if (steeringMagnitude > s_maxSteeringMagnitude)
            steeringForce *= s_maxSteeringMagnitude / steeringMagnitude;
        else if (steeringMagnitude < s_minSteeringMagnitude)
            steeringForce *= s_minSteeringMagnitude / steeringMagnitude;

        updatedVelocity = primaryBoid.m_velocity + steeringForce * deltaTime;

        // Min/Max clamp for velocity
        const float velocityMagnitude{ glm::length(updatedVelocity) };
        if (velocityMagnitude > s_maxSpeed)
            updatedVelocity *= s_maxSpeed / velocityMagnitude;
        else if (velocityMagnitude < s_minSpeed)
            updatedVelocity *= s_minSpeed / velocityMagnitude;

        updatedVelocities[i] = updatedVelocity;
    }

    for (size_t i{ 0 }; i < s_boids.size(); ++i)
    {
        Boid& boid{ s_boids[i] };

        boid.m_velocity = updatedVelocities[i];
        boid.m_pos += boid.m_velocity * deltaTime;
        if (boid.m_pos.y - s_triangleHeight > Camera::screenHeight)
            boid.m_pos.y -= Camera::screenHeight + s_triangleHeight;
        if (boid.m_pos.y + s_triangleHeight < 0)
            boid.m_pos.y += Camera::screenHeight + s_triangleHeight;
        if (boid.m_pos.x - s_triangleHeight > Camera::screenWidth)
            boid.m_pos.x -= Camera::screenWidth + s_triangleHeight;
        if (boid.m_pos.x + s_triangleHeight < 0)
            boid.m_pos.x += Camera::screenWidth + s_triangleHeight;
    }
}

void Boid::createBoid(glm::vec2 pos)
{
    s_boids.emplace_back(pos);
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;
    m_velocity = glm::vec2{ rd::speedDistribution(rd::randomNumberGenerator), rd::speedDistribution(rd::randomNumberGenerator) };
}
