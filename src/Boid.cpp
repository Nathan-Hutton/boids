#include "Boid.h"
#include "Camera.h"

#include "imgui.h"

#include <random>
#include <vector>
#include <array>
#include <iostream>

std::vector<Boid> Boid::s_boids{};
GLuint Boid::s_VAO{};
float Boid::s_triangleWidth{};
float Boid::s_triangleHeight{};

float Boid::s_maxSpeed{};
float Boid::s_minSpeed{};
float Boid::s_minSteeringMagnitude{};

float Boid::s_radius{};
float Boid::s_visionAngleCos{};

namespace rd
{
    std::random_device rd;
    std::mt19937 randomNumberGenerator{rd()};
    std::uniform_real_distribution<float> speedDistribution{};
}

namespace ui
{
    float radiusScale{ 1.0f / 20.0f };
    float visionAngleDegrees{ 270.0f };
    float maxSpeedScale{ 1.0f / 6.0f };
    float minSpeedScale{ 1.0f / 14.0f };
    float minSteeringMagnitudeScale{ 1.0f / 8.5f };

    float alignmentScale{ 5.0f };

    namespace visionCone
    {
        GLuint VAO{};
        GLuint VBO{};
        std::array<GLfloat, 102> vertices{}; // So really 51 vertices (including middle)
    }
}

void Boid::recomputeStaticParams()
{
    s_radius = Camera::screenWidth * ui::radiusScale;
    s_visionAngleCos = glm::cos(glm::radians(ui::visionAngleDegrees) / 2.0f);

    s_maxSpeed = Camera::screenWidth * ui::maxSpeedScale;
    s_minSpeed = Camera::screenWidth * ui::minSpeedScale;
    s_minSteeringMagnitude = Camera::screenWidth * ui::minSteeringMagnitudeScale;

    rd::speedDistribution = std::uniform_real_distribution<float>{-s_maxSpeed / 5.0f, s_maxSpeed / 5.0f};
}

void Boid::init()
{
    s_triangleWidth = Camera::screenWidth / 220.0f;
    s_triangleHeight = Camera::screenHeight / 80.0f;
    recomputeStaticParams();

    // Boid triangle VAO
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

    // Boid vision cone/circle VAO
    ui::visionCone::vertices[0] = 0.0f;
    ui::visionCone::vertices[1] = 0.0f;

    const GLfloat stepSize{ glm::radians(ui::visionAngleDegrees / (static_cast<float>(ui::visionCone::vertices.size() / 2) - 1.0f)) };
    const GLfloat startAngle{ glm::radians(((360.0f - ui::visionAngleDegrees) / 2.0f) - 90.0f) };
    size_t index{ 2 };
    for (size_t i{ 0 }; i < (ui::visionCone::vertices.size() - 2) / 2; ++i)
    {
        const GLfloat x{glm::cos(startAngle + (stepSize * static_cast<float>(i))) * s_radius};
        const GLfloat y{glm::sin(startAngle + (stepSize * static_cast<float>(i))) * s_radius};
        ui::visionCone::vertices[index++] = x;
        ui::visionCone::vertices[index++] = y;
    }

    glGenVertexArrays(1, &ui::visionCone::VAO);
    glBindVertexArray(ui::visionCone::VAO);

    glGenBuffers(1, &ui::visionCone::VBO);
    glBindBuffer(GL_ARRAY_BUFFER, ui::visionCone::VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ui::visionCone::vertices), ui::visionCone::vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Boid::showImGuiControls()
{
    bool changed{ false };
    changed |= ImGui::SliderFloat("Radius scale", &ui::radiusScale, 0.0f, 0.3f);
    changed |= ImGui::SliderFloat("Vision angle (degrees)", &ui::visionAngleDegrees, 0.0f, 360.0f);
    changed |= ImGui::SliderFloat("Max speed scale", &ui::maxSpeedScale, ui::minSpeedScale, 0.8f );
    changed |= ImGui::SliderFloat("Min speed scale", &ui::minSpeedScale, 1.0f / 50.0f, ui::maxSpeedScale );
    changed |= ImGui::SliderFloat("Min steering force scale", &ui::minSteeringMagnitudeScale, 0.05f, 0.8f );

    ImGui::SliderFloat("Alignment scale", &ui::alignmentScale, 0.0f, 20.0f);

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
            steeringForce *= ui::alignmentScale;
        }

        float steeringMagnitude{ glm::length(steeringForce) };
        while (steeringMagnitude < 1e-6)
        {
            steeringForce = glm::vec2{ rd::speedDistribution(rd::randomNumberGenerator) / 5.0f, rd::speedDistribution(rd::randomNumberGenerator) / 5.0f };
            steeringMagnitude = glm::length(steeringForce);
        }

        // Min clamp for steering force (so things don't move too slow)
        if (steeringMagnitude < s_minSteeringMagnitude)
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

void Boid::renderBoid()
{
    glBindVertexArray(s_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(ui::visionCone::VAO);
    glDrawArrays(GL_LINE_LOOP, 0, ui::visionCone::vertices.size());
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;
    m_velocity = glm::vec2{ rd::speedDistribution(rd::randomNumberGenerator), rd::speedDistribution(rd::randomNumberGenerator) };
}
