#include "Boid.h"
#include "BoidParams.h"
#include "../../Camera.h"
#include "../../ShaderHandler.h"
#include "../UI.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <array>
#include <iostream>

namespace
{
    glm::vec3 getRGBFromHue(float hue)
    {
        const float h{ hue * 6.0f };
        const int i{ int(floor(h)) };
        const float f{ h - i };
        const float p{ simulation::boid::globalVars::brightness * (1.0f - simulation::boid::globalVars::saturation) };
        const float q{ simulation::boid::globalVars::brightness * (1.0f - simulation::boid::globalVars::saturation * f) };
        const float t{ simulation::boid::globalVars::brightness * (1.0f - simulation::boid::globalVars::saturation * (1.0f - f)) };

        switch (i % 6)
        {
            case 0 : return { simulation::boid::globalVars::brightness, t, q };
            case 1 : return { q, simulation::boid::globalVars::brightness, p };
            case 2 : return { p, simulation::boid::globalVars::brightness, t };
            case 3 : return { p, q, simulation::boid::globalVars::brightness };
            case 4 : return { t, p, simulation::boid::globalVars::brightness };
            case 5 : return { simulation::boid::globalVars::brightness, p, q };
        }

        return { 0.0f, 0.0f, 0.0f };
    }
}

std::vector<simulation::boid::Boid> simulation::boid::Boid::s_boids{};

void simulation::boid::Boid::updateBoids(float deltaTime)
{
    std::vector<glm::vec2> updatedVelocities(s_boids.size());

    // First, we filter out the neighboring boids that aren't actually neighbors (must be within radius and vision angle/cone)
    for (size_t i{ 0 }; i < s_boids.size(); ++i)
    {
        Boid& primaryBoid{ s_boids[i] };
        int numVisibleBoids{ 0 };
        glm::vec2 updatedVelocity{ primaryBoid.m_velocity };

        glm::vec2 steeringForce{ 0.0f };
        glm::vec2 noise{ globalVars::rd::centeredDistribution(globalVars::rd::randomNumberGenerator) * (globalVars::maxSpeed * 1.5f), globalVars::rd::centeredDistribution(globalVars::rd::randomNumberGenerator) * (globalVars::maxSpeed * 1.5f) };
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
            if (distance > globalVars::visionRadius || distance < 1e-6) continue;

            const glm::vec2 dirToOther{ glm::normalize(vecToOther) };
            if (glm::dot(glm::normalize(primaryBoid.m_velocity), dirToOther) < globalVars::visionAngleCos) continue;
            ++numVisibleBoids;

            const float strength{ glm::clamp((globalVars::visionRadius - distance) / globalVars::visionRadius, 0.0f, 1.0f) };
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
        separationForce *= globalVars::separation;

        // Alignment
        alignmentForce *= globalVars::alignment;

        // Cohesion
        cohesionForce /= numVisibleBoids;
        cohesionForce = (cohesionForce - primaryBoid.m_pos) * globalVars::cohesion;

        // Update positions and velocities
        steeringForce += separationForce + alignmentForce + cohesionForce + noise;
        updatedVelocity = primaryBoid.m_velocity + steeringForce * deltaTime;

        if (glm::length(updatedVelocity) > globalVars::maxSpeed)
            updatedVelocity = glm::normalize(updatedVelocity) * globalVars::maxSpeed;

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

        if (globalVars::saturation < 0.7f)
        {
            const float hueNoise{ globalVars::rd::centeredDistribution(globalVars::rd::randomNumberGenerator) };
            primaryBoid.m_hue += hueNoise * deltaTime;
        }

        primaryBoid.m_hue = std::fmod(primaryBoid.m_hue + 1.0f, 1.0f);
    }

    for (size_t i{ 0 }; i < s_boids.size(); ++i)
    {
        Boid& boid{ s_boids[i] };

        boid.m_velocity = updatedVelocities[i];

        boid.m_pos += boid.m_velocity * deltaTime;
        if (boid.m_pos.y - globalVars::triangleHeight > Camera::screenHeight)
            boid.m_pos.y -= Camera::screenHeight + globalVars::triangleHeight;
        if (boid.m_pos.y + globalVars::triangleHeight < 0)
            boid.m_pos.y += Camera::screenHeight + globalVars::triangleHeight;
        if (boid.m_pos.x - globalVars::triangleHeight > Camera::screenWidth)
            boid.m_pos.x -= Camera::screenWidth + globalVars::triangleHeight;
        if (boid.m_pos.x + globalVars::triangleHeight < 0)
            boid.m_pos.x += Camera::screenWidth + globalVars::triangleHeight;
    }
}

void simulation::boid::Boid::createBoid(glm::vec2 pos)
{
    for (int i{ 0 }; i < ui::numBoidsPerClick; ++i)
        s_boids.emplace_back(pos);
}

void simulation::boid::Boid::renderAllBoids()
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

    glBindVertexArray(globalVars::VAO);
    for (const Boid& boid : s_boids)
    {
        glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(getRGBFromHue(boid.m_hue)));

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
            glPointSize(globalVars::triangleWidth / 1.5f);
            glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ boid.m_pos, 0.0f }) };
            model = glm::rotate(model, boid.getRotation(), glm::vec3{ 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_POINTS, 0, 1);
        }
    }
}

simulation::boid::Boid::Boid(glm::vec2 pos)
    : m_pos{ pos }
    , m_velocity{ glm::vec2{ 
        globalVars::rd::centeredDistribution(globalVars::rd::randomNumberGenerator),
        globalVars::rd::centeredDistribution(globalVars::rd::randomNumberGenerator) 
    } * (globalVars::maxSpeed * 0.25f) }
    , m_hue{ (globalVars::rd::centeredDistribution(globalVars::rd::randomNumberGenerator) + 1.0f) / 2.0f }
{}
