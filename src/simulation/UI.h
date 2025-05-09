#pragma once

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
}
