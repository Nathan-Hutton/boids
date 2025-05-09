#pragma once

#include "boid/BoidParams.h"
#include "../Camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <array>

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
            changed |= ImGui::SliderFloat("Separation scale", &separationScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##SeparationInput", &separationScale, 1.0f);
            separationScale = std::clamp(separationScale, 0.0f, 8.0f);

            // Alignment scale
            changed |= ImGui::SliderFloat("Alignment scale", &alignmentScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##AlignmentInput", &alignmentScale, 1.0f);
            alignmentScale = std::clamp(alignmentScale, 0.0f, 8.0f);

            // Cohesion scale
            changed |= ImGui::SliderFloat("Cohesion scale", &cohesionScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##CohesionInput", &cohesionScale, 1.0f);
            cohesionScale = std::clamp(cohesionScale, 0.0f, 8.0f);

            // Max speed scale
            changed |= ImGui::SliderFloat("Max speed scale", &maxSpeedScale, 0.0f, 4.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##MaxSpeedInput", &maxSpeedScale, 0.5f);
            maxSpeedScale = std::clamp(maxSpeedScale, 0.0f, 4.0f);
        }

        if (ImGui::CollapsingHeader("Radius")) 
        {
            ImGui::Checkbox("Show vision cones", &showVisionCones);

            changed |= ImGui::SliderFloat("Radius scale", &visionRadiusScale, 0.0f, 8.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##RadiusInput", &visionRadiusScale, 1.0f);
            visionRadiusScale = std::clamp(visionRadiusScale, 0.0f, 8.0f);

            changed |= ImGui::SliderFloat("Vision angle (degrees)", &visionAngleDegrees, 0.0f, 360.0f);
            ImGui::SameLine();
            changed |= ImGui::InputFloat("##VisionAngleInput", &visionAngleDegrees, 5.0f);
            visionAngleDegrees = std::clamp(visionAngleDegrees, 0.0f, 360.0f);
        }

        if (changed)
            boid::recomputeGlobalBoidParams();

        if (ImGui::CollapsingHeader("Scene")) 
        {
            ImGui::SliderInt("Boids per click", &numBoidsPerClick, 1, 100);
            ImGui::SameLine();
            ImGui::InputInt("##NumBoidsInput", &numBoidsPerClick, 1);
            numBoidsPerClick = std::clamp(numBoidsPerClick, 1, 100);

            if (ImGui::Button("Clear boids"))
                boid::boids.clear();
        }

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
