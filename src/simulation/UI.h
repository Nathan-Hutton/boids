#pragma once

#include "boid/BoidParams.h"
#include "../Camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <array>
#include <iostream>

namespace simulation::ui
{
    // *****
    // Boids
    // *****
    inline float separationScale{ 1.0f };
    inline float alignmentScale{ 1.0f };
    inline float cohesionScale{ 1.0f };

    inline float visionRadiusScale{ 1.0f };
    inline float visionAngleDegrees{ 270.0f };
    inline float maxSpeedScale{ 1.0f };

    inline bool showVisionCones{ false };
    inline GLuint visionConeVAO{};
    inline GLuint visionConeVBO{};
    inline std::array<GLfloat, 104> visionConeVertices{ 0.0f }; // So really 51 vertices (including middle)

    inline int numBoidsPerClick{ 1 };

    // *********
    // Obstacles
    // *********
    inline float obstacleRadiusScale{ 1.0f };

    inline void renderUI()
    {
        ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushItemWidth(Camera::screenWidth / 20.0f);
        bool changed{ false };

        if (ImGui::CollapsingHeader("Primary")) 
        {
            // Separation scale
            changed = ImGui::SliderFloat("Separation scale", &separationScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##SeparationInput", &separationScale, 1.0f);
            if (changed)
            {
                separationScale = std::clamp(separationScale, 0.0f, 8.0f);
                boid::separation = boid::defaultSeparation * separationScale;
            }

            // Alignment scale
            changed = ImGui::SliderFloat("Alignment scale", &alignmentScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##AlignmentInput", &alignmentScale, 1.0f);
            if (changed)
            {
                alignmentScale = std::clamp(alignmentScale, 0.0f, 8.0f);
                boid::alignment = boid::defaultAlignment * alignmentScale;
            }

            // Cohesion scale
            changed = ImGui::SliderFloat("Cohesion scale", &cohesionScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##CohesionInput", &cohesionScale, 1.0f);
            if (changed)
            {
                cohesionScale = std::clamp(cohesionScale, 0.0f, 8.0f);
                boid::cohesion = boid::defaultCohesion * cohesionScale;
            }

            // Max speed scale
            changed |= ImGui::SliderFloat("Max speed scale", &maxSpeedScale, 0.0f, 4.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##MaxSpeedInput", &maxSpeedScale, 0.5f);
            if (changed)
            {
                maxSpeedScale = std::clamp(maxSpeedScale, 0.0f, 4.0f);
                boid::maxSpeed = boid::defaultMaxSpeed * maxSpeedScale;
            }
        }

        if (ImGui::CollapsingHeader("Radius")) 
        {
            ImGui::Checkbox("Show vision cones", &showVisionCones);

            changed = ImGui::SliderFloat("Radius scale", &visionRadiusScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##RadiusInput", &visionRadiusScale, 1.0f);
            if (changed)
            {
                visionRadiusScale = std::clamp(visionRadiusScale, 0.0f, 8.0f);
                boid::visionRadius = boid::defaultVisionRadius * visionRadiusScale;
                boid::recomputeVisionConeVBO();
            }

            changed = ImGui::SliderFloat("Vision angle (degrees)", &visionAngleDegrees, 0.0f, 360.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##VisionAngleInput", &visionAngleDegrees, 5.0f);
            if (changed)
            {
                visionAngleDegrees = std::clamp(visionAngleDegrees, 0.0f, 360.0f);
                boid::visionAngleCos = glm::cos(glm::radians(visionAngleDegrees) / 2.0f);
                boid::recomputeVisionConeVBO();
            }
        }

        // I'm not doing the changed bool thing here since there's not many computations going on
        if (ImGui::CollapsingHeader("Scene")) 
        {
            ImGui::SliderInt("Boids per click", &numBoidsPerClick, 1, 100);
            ImGui::SameLine();
            ImGui::InputInt("##NumBoidsInput", &numBoidsPerClick, 1);
            numBoidsPerClick = std::clamp(numBoidsPerClick, 1, 100);

            if (ImGui::Button("Clear boids"))
                boid::boids.clear();
        }

        // I'm not doing the changed bool thing here since there's not many computations going on
        if (ImGui::CollapsingHeader("Color")) 
        {
            ImGui::SliderFloat("Saturation", &boid::saturation, 0.003f, 1.0f); // Not letting it go to zero to avoid a zero division error in updateBoids
            ImGui::SameLine();
            ImGui::InputFloat("##SaturationInput", &boid::saturation, 1.0f);
            boid::saturation = std::clamp(boid::saturation, 0.003f, 1.0f);

            ImGui::SliderFloat("Brightness", &boid::brightness, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::InputFloat("##BrightnessInput", &boid::brightness, 1.0f);
            boid::brightness = std::clamp(boid::brightness, 0.0f, 1.0f);

            if (ImGui::Button("Randomize Hues"))
                boid::randomizeHues();
        }

        ImGui::PopItemWidth();
        ImGui::End();
    }
}
