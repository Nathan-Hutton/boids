#include "Boid.h"
#include "Camera.h"
#include "ShaderHandler.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <vector>
#include <array>
#include <iostream>

std::vector<Boid> Boid::s_boids{};
GLuint Boid::s_VAO{};
float Boid::s_triangleWidth{};
float Boid::s_triangleHeight{};

float Boid::s_radius{};
float Boid::s_visionAngleCos{};

namespace rd
{
    std::random_device rd;
    std::mt19937 randomNumberGenerator{rd()};
    std::uniform_real_distribution<float> distribution{};
}

namespace settings
{
    float alignmentScale{ 5.0f };
    float radiusScale{ 1.0f / 20.0f };
    float visionAngleDegrees{ 270.0f };

    namespace visionCone
    {
        bool showVisionCones{ false };
        GLuint VAO{};
        GLuint VBO{};
        std::array<GLfloat, 102> vertices{ 0.0f }; // So really 51 vertices (including middle)
    }
}

void Boid::recomputeStaticParams()
{
    s_radius = Camera::screenWidth * settings::radiusScale;
    s_visionAngleCos = glm::cos(glm::radians(settings::visionAngleDegrees) / 2.0f);

    rd::distribution = std::uniform_real_distribution<float>{-1.0f, 1.0f};

    // Recompute vision cone vertices
    const GLfloat stepSize{ glm::radians(settings::visionAngleDegrees / (static_cast<float>(settings::visionCone::vertices.size() / 2) - 1.0f)) };
    const GLfloat startAngle{ glm::radians(((360.0f - settings::visionAngleDegrees) / 2.0f) - 90.0f) };
    size_t index{ 2 };
    for (size_t i{ 0 }; i < (settings::visionCone::vertices.size() - 2) / 2; ++i)
    {
        const GLfloat x{glm::cos(startAngle + (stepSize * static_cast<float>(i))) * s_radius};
        const GLfloat y{glm::sin(startAngle + (stepSize * static_cast<float>(i))) * s_radius};
        settings::visionCone::vertices[index++] = x;
        settings::visionCone::vertices[index++] = y;
    }

    glBindBuffer(GL_ARRAY_BUFFER, settings::visionCone::VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 2, sizeof(settings::visionCone::vertices) - (sizeof(settings::visionCone::vertices[0]) * 2.0f), settings::visionCone::vertices.data());
    glBufferData(GL_ARRAY_BUFFER, sizeof(settings::visionCone::vertices), settings::visionCone::vertices.data(), GL_DYNAMIC_DRAW);
}

void Boid::init()
{
    s_triangleWidth = Camera::screenWidth / 220.0f;
    s_triangleHeight = Camera::screenHeight / 80.0f;
    glPointSize(s_triangleWidth / 1.5f); // The points will show up at m_pos when we render the cones so we can see exactly where the boids are visible
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

    // Boid vision cone/circle VAO. This is already initialized to zero
    glGenVertexArrays(1, &settings::visionCone::VAO);
    glBindVertexArray(settings::visionCone::VAO);

    glGenBuffers(1, &settings::visionCone::VBO);
    glBindBuffer(GL_ARRAY_BUFFER, settings::visionCone::VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(settings::visionCone::vertices), settings::visionCone::vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Boid::showImGuiControls()
{
    ImGui::Checkbox("Show vision cones", &settings::visionCone::showVisionCones);
    ImGui::SliderFloat("Alignment scale", &settings::alignmentScale, 0.0f, 20.0f);

    bool changed{ false };
    changed |= ImGui::SliderFloat("Radius scale", &settings::radiusScale, 0.0f, 0.3f);
    changed |= ImGui::SliderFloat("Vision angle (degrees)", &settings::visionAngleDegrees, 0.0f, 360.0f);

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
        glm::vec2 updatedVelocity{ primaryBoid.m_velocity };

        // Noise (this will be the entire steering force if there are 0 neighbors)
        glm::vec2 steeringForce{ rd::distribution(rd::randomNumberGenerator) * 5.0f, rd::distribution(rd::randomNumberGenerator) * 5.0f };

        // Alignment
        glm::vec2 alignmentForce{ 0.0f };
        glm::vec2 avgNeighborVelocity{ 0.0f };

        // Cohesion
        glm::vec2 cohesionForce{ 0.0f };

        // Separation
        glm::vec2 separationForce{ 0.0f };

        for (size_t j{ 0 }; j < s_boids.size(); ++j)
        {
            if (i == j) continue;
            const Boid& otherBoid{ s_boids[j] };

            glm::vec2 vecToOther{ otherBoid.m_pos - primaryBoid.m_pos };

            // Account for wraparound
            if (std::abs(vecToOther.x) > Camera::screenWidth / 2.0f)
                vecToOther.x -= glm::sign(vecToOther.x) * Camera::screenWidth;
            if (std::abs(vecToOther.y) > Camera::screenHeight / 2.0f)
                vecToOther.y -= glm::sign(vecToOther.y) * Camera::screenHeight;

            const float distance{ glm::length(vecToOther) };
            if (distance > s_radius || distance < 1e-6) continue;

            const glm::vec2 dirToOther{ glm::normalize(vecToOther) };
            if (glm::dot(glm::normalize(primaryBoid.m_velocity), dirToOther) < s_visionAngleCos) continue;

            ++numVisibleBoids;
            avgNeighborVelocity += otherBoid.m_velocity;
        }

        if (numVisibleBoids == 0)
        {
            updatedVelocity = primaryBoid.m_velocity + steeringForce * deltaTime;
            updatedVelocities[i] = updatedVelocity;
            continue;
        }

        if (numVisibleBoids > 0)
        {
            // *********
            // Alignment
            // *********
            avgNeighborVelocity /= numVisibleBoids;

            steeringForce += avgNeighborVelocity - primaryBoid.m_velocity;
            steeringForce *= settings::alignmentScale;
        }

        updatedVelocity = primaryBoid.m_velocity + steeringForce * deltaTime;
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

void Boid::renderAllBoids()
{
    // Remember that depth testing is off
    // First, render the vision cones, then the outlines, then the boids themselves, and finally m_pos as points
    if (settings::visionCone::showVisionCones)
    {
        glBindVertexArray(settings::visionCone::VAO);

        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 0.1f, 0.1f, 0.1f }));
        for (const Boid& boid : s_boids)
        {
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.getPos(), 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_TRIANGLE_FAN, 0, settings::visionCone::vertices.size());
        }

        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 0.35f, 0.35f, 0.35f }));
        for (const Boid& boid : s_boids)
        {
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.getPos(), 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_LINE_LOOP, 0, settings::visionCone::vertices.size());
        }
    }

    glBindVertexArray(s_VAO);
    glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 0.0f, 1.0f, 1.0f }));
    for (const Boid& boid : s_boids)
    {
        glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.getPos(), 0.0f }) };
        model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
        glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // Render m_pos as a point so we can see exactly where the boids are visible in a render cone
    if (settings::visionCone::showVisionCones)
    {
        glBindVertexArray(settings::visionCone::VAO);
        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 1.0f, 0.0f, 0.0f }));
        for (const Boid& boid : s_boids)
        {
            glPointSize(s_triangleWidth / 1.5f);
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.getPos(), 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_POINTS, 0, 1);
        }
    }
}

Boid::Boid(glm::vec2 pos)
{
    m_pos = pos;
    m_velocity = glm::vec2{ rd::distribution(rd::randomNumberGenerator), rd::distribution(rd::randomNumberGenerator) } * (s_triangleWidth * 5.0f);
}
