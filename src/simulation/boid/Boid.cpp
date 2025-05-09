#include "Boid.h"
#include "../../Camera.h"
#include "../../ShaderHandler.h"
#include "../UI.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <array>
#include <iostream>

using Boid = simulation::boid::Boid;
namespace ui = simulation::ui;

std::vector<Boid> Boid::s_boids{};
GLuint Boid::s_VAO{};
float Boid::s_triangleWidth{};
float Boid::s_triangleHeight{};

float Boid::s_separation{};
float Boid::s_alignment{};
float Boid::s_cohesion;

float Boid::s_maxSpeed{};

float Boid::s_radius{};
float Boid::s_visionAngleCos{};

namespace color
{
    float saturation{ 90.0f / 255.0f };
    float brightness{ 200.0f / 255.0f };

    glm::vec3 getRGBFromHue(float hue)
    {
        const float h{ hue * 6.0f };
        const int i{ int(floor(h)) };
        const float f{ h - i };
        const float p{ brightness * (1.0f - saturation) };
        const float q{ brightness * (1.0f - saturation * f) };
        const float t{ brightness * (1.0f - saturation * (1.0f - f)) };

        switch (i % 6)
        {
            case 0 : return { brightness, t, q };
            case 1 : return { q, brightness, p };
            case 2 : return { p, brightness, t };
            case 3 : return { p, q, brightness };
            case 4 : return { t, p, brightness };
            case 5 : return { brightness, p, q };
        }

        return { 0.0f, 0.0f, 0.0f };
    }
}

namespace rd
{
    std::random_device rd;
    std::mt19937 randomNumberGenerator{rd()};
    std::uniform_real_distribution<float> centeredDistribution{-1.0f, 1.0f};
}

void Boid::recomputeStaticParams()
{
    s_separation = ui::separationScale * (Camera::screenWidth * 0.15f);
    s_alignment = ui::alignmentScale * (Camera::screenWidth * 0.15f);
    s_cohesion = ui::cohesionScale * (Camera::screenWidth / 240.0f);

    s_maxSpeed = (Camera::screenWidth / 10.0f) * ui::maxSpeedScale;

    s_radius = (Camera::screenWidth / 20.0f) * ui::visionRadiusScale;
    s_visionAngleCos = glm::cos(glm::radians(ui::visionAngleDegrees) / 2.0f);

    // Recompute vision cone vertices
    const size_t numSegments{ (ui::visionConeVertices.size() - 4) / 2 };
    const GLfloat stepSize{ glm::radians(ui::visionAngleDegrees / static_cast<float>(numSegments)) };
    const GLfloat startAngle{ glm::radians(((360.0f - ui::visionAngleDegrees) / 2.0f) - 90.0f) };

    size_t index{ 2 };
    for (size_t i{ 0 }; i <= numSegments; ++i)
    {
        const GLfloat x{glm::cos(startAngle + (stepSize * static_cast<float>(i))) * s_radius};
        const GLfloat y{glm::sin(startAngle + (stepSize * static_cast<float>(i))) * s_radius};
        ui::visionConeVertices[index++] = x;
        ui::visionConeVertices[index++] = y;
    }

    glBindBuffer(GL_ARRAY_BUFFER, ui::visionConeVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ui::visionConeVertices), ui::visionConeVertices.data());
}

void Boid::randomizeHues()
{
    for (Boid& boid : s_boids)
        boid.m_hue = (rd::centeredDistribution(rd::randomNumberGenerator) + 1.0f) / 2.0f;
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
    glGenVertexArrays(1, &ui::visionConeVAO);
    glBindVertexArray(ui::visionConeVAO);

    glGenBuffers(1, &ui::visionConeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ui::visionConeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ui::visionConeVertices), ui::visionConeVertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Boid::showImGuiControls()
{
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(Camera::screenWidth / 20.0f);
    bool changed{ false };

    if (ImGui::CollapsingHeader("Primary")) 
    {
        // Separation scale
        changed |= ImGui::SliderFloat("Separation scale", &ui::separationScale, 0.0f, 8.0f);
        ImGui::SameLine();
        changed |= ImGui::InputFloat("##SeparationInput", &ui::separationScale, 1.0f);
        ui::separationScale = std::clamp(ui::separationScale, 0.0f, 8.0f);

        // Alignment scale
        changed |= ImGui::SliderFloat("Alignment scale", &ui::alignmentScale, 0.0f, 8.0f);
        ImGui::SameLine();
        changed |= ImGui::InputFloat("##AlignmentInput", &ui::alignmentScale, 1.0f);
        ui::alignmentScale = std::clamp(ui::alignmentScale, 0.0f, 8.0f);

        // Cohesion scale
        changed |= ImGui::SliderFloat("Cohesion scale", &ui::cohesionScale, 0.0f, 8.0f);
        ImGui::SameLine();
        changed |= ImGui::InputFloat("##CohesionInput", &ui::cohesionScale, 1.0f);
        ui::cohesionScale = std::clamp(ui::cohesionScale, 0.0f, 8.0f);

        // Max speed scale
        changed |= ImGui::SliderFloat("Max speed scale", &ui::maxSpeedScale, 0.0f, 4.0f);
        ImGui::SameLine();
        changed |= ImGui::InputFloat("##MaxSpeedInput", &ui::maxSpeedScale, 0.5f);
        ui::maxSpeedScale = std::clamp(ui::maxSpeedScale, 0.0f, 4.0f);
    }

    if (ImGui::CollapsingHeader("Radius")) 
    {
        ImGui::Checkbox("Show vision cones", &ui::showVisionCones);

        changed |= ImGui::SliderFloat("Radius scale", &ui::visionRadiusScale, 0.0f, 8.0f);
        ImGui::SameLine();
        changed |= ImGui::InputFloat("##RadiusInput", &ui::visionRadiusScale, 1.0f);
        ui::visionRadiusScale = std::clamp(ui::visionRadiusScale, 0.0f, 8.0f);

        changed |= ImGui::SliderFloat("Vision angle (degrees)", &ui::visionAngleDegrees, 0.0f, 360.0f);
        ImGui::SameLine();
        changed |= ImGui::InputFloat("##VisionAngleInput", &ui::visionAngleDegrees, 5.0f);
        ui::visionAngleDegrees = std::clamp(ui::visionAngleDegrees, 0.0f, 360.0f);
    }

    if (changed)
        recomputeStaticParams();

    if (ImGui::CollapsingHeader("Scene")) 
    {
        ImGui::SliderInt("Boids per click", &ui::numBoidsPerClick, 1, 100);
        ImGui::SameLine();
        ImGui::InputInt("##NumBoidsInput", &ui::numBoidsPerClick, 1);
        ui::numBoidsPerClick = std::clamp(ui::numBoidsPerClick, 1, 100);

        if (ImGui::Button("Clear boids"))
            s_boids.clear();
    }

    if (ImGui::CollapsingHeader("Color")) 
    {
        ImGui::SliderFloat("Saturation", &color::saturation, 0.003f, 1.0f); // Not letting it go to zero to avoid a zero division error in updateBoids
        ImGui::SameLine();
        ImGui::InputFloat("##SaturationInput", &color::saturation, 1.0f);
        color::saturation = std::clamp(color::saturation, 0.003f, 1.0f);

        ImGui::SliderFloat("Brightness", &color::brightness, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::InputFloat("##BrightnessInput", &color::brightness, 1.0f);
        color::brightness = std::clamp(color::brightness, 0.0f, 1.0f);

        if (ImGui::Button("Randomize Hues"))
            randomizeHues();
    }

    ImGui::PopItemWidth();
    ImGui::End();
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

        glm::vec2 steeringForce{ 0.0f };
        glm::vec2 noise{ rd::centeredDistribution(rd::randomNumberGenerator) * (s_maxSpeed * 1.5f), rd::centeredDistribution(rd::randomNumberGenerator) * (s_maxSpeed * 1.5f) };
        steeringForce += noise;

        glm::vec2 separationForce{ 0.0f };
        glm::vec2 alignmentForce{ 0.0f };
        glm::vec2 cohesionForce{ 0.0f };

        float hueSinSum{ 0.0f };
        float hueCosSum{ 0.0f };

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

            const float strength{ glm::clamp((s_radius - distance) / s_radius, 0.0f, 1.0f) };
            separationForce += -dirToOther * strength;

            //alignmentForce += otherBoid.m_velocity * strength;
            alignmentForce += glm::normalize(otherBoid.m_velocity) * strength;

            cohesionForce += primaryBoid.m_pos + vecToOther; // I'm doing this instead of using neighborBoid.m_pos to account for wraparound

            // Hue averaging using complex number projection
            const float hueAngle{ glm::two_pi<float>() * otherBoid.m_hue };
            hueSinSum += sin(hueAngle);
            hueCosSum += cos(hueAngle);
        }

        if (numVisibleBoids == 0)
        {
            updatedVelocity = primaryBoid.m_velocity + steeringForce * deltaTime;
            updatedVelocities[i] = updatedVelocity;
            continue;
        }

        // Separation
        separationForce *= s_separation;

        // Alignment
        alignmentForce *= s_alignment;

        // Cohesion
        cohesionForce /= numVisibleBoids;
        cohesionForce = (cohesionForce - primaryBoid.m_pos) * s_cohesion;

        // Update positions and velocities
        steeringForce += separationForce + alignmentForce + cohesionForce + noise;
        updatedVelocity = primaryBoid.m_velocity + steeringForce * deltaTime;

        if (glm::length(updatedVelocity) > s_maxSpeed)
            updatedVelocity = glm::normalize(updatedVelocity) * s_maxSpeed;

        updatedVelocities[i] = updatedVelocity;

        // Update hue
        float avgHueAngle{ static_cast<float>(atan2(hueSinSum, hueCosSum)) };
        if (avgHueAngle < 0.0f)
            avgHueAngle += glm::two_pi<float>();

        const float avgHue{ avgHueAngle / glm::two_pi<float>() };

        float hueDelta{ avgHue - primaryBoid.m_hue };
        if (hueDelta > 0.5f) 
            hueDelta -= 1.0f;
        else if (hueDelta < -0.5f) 
            hueDelta += 1.0f;

        primaryBoid.m_hue += hueDelta * 3.0f * deltaTime;

        if (color::saturation < 0.7f)
        {
            const float hueNoise{ rd::centeredDistribution(rd::randomNumberGenerator) };
            primaryBoid.m_hue += hueNoise * deltaTime;
        }

        primaryBoid.m_hue = std::fmod(primaryBoid.m_hue + 1.0f, 1.0f);
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
    for (int i{ 0 }; i < ui::numBoidsPerClick; ++i)
        s_boids.emplace_back(pos);
}

void Boid::renderAllBoids()
{
    // Remember that depth testing is off
    // First, render the vision cones, then the outlines, then the boids themselves, and finally m_pos as points
    if (ui::showVisionCones)
    {
        glBindVertexArray(ui::visionConeVAO);

        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 0.1f, 0.1f, 0.1f }));
        for (const Boid& boid : s_boids)
        {
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.m_pos, 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_TRIANGLE_FAN, 0, ui::visionConeVertices.size());
        }

        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 0.35f, 0.35f, 0.35f }));
        for (const Boid& boid : s_boids)
        {
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.m_pos, 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_LINE_LOOP, 0, ui::visionConeVertices.size());
        }
    }

    glBindVertexArray(s_VAO);
    for (const Boid& boid : s_boids)
    {
        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(color::getRGBFromHue(boid.m_hue)));

        glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.m_pos, 0.0f }) };
        model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
        glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // Render m_pos as a point so we can see exactly where the boids are visible in a render cone
    if (ui::showVisionCones)
    {
        glBindVertexArray(ui::visionConeVAO);
        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 1.0f, 0.0f, 0.0f }));
        for (const Boid& boid : s_boids)
        {
            glPointSize(s_triangleWidth / 1.5f);
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.m_pos, 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_POINTS, 0, 1);
        }
    }
}

Boid::Boid(glm::vec2 pos)
    : m_pos{ pos }
    , m_velocity{ glm::vec2{ 
        rd::centeredDistribution(rd::randomNumberGenerator),
        rd::centeredDistribution(rd::randomNumberGenerator) 
    } * (s_maxSpeed * 0.25f) }
    , m_hue{ (rd::centeredDistribution(rd::randomNumberGenerator) + 1.0f) / 2.0f }
{}
