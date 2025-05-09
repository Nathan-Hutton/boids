#pragma once

#include "../../Camera.h"
#include "../UI.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

namespace simulation::boid
{
    inline std::vector<Boid> boids{};
    inline GLuint VAO{};
    inline float triangleWidth{};
    inline float triangleHeight{};

    inline float separation{};
    inline float alignment{};
    inline float cohesion;

    inline float maxSpeed{};

    inline float visionRadius{};
    inline float visionAngleCos{};

    inline void recomputeStaticParams()
    {
        separation = ui::separationScale * (Camera::screenWidth * 0.15f);
        alignment = ui::alignmentScale * (Camera::screenWidth * 0.15f);
        cohesion = ui::cohesionScale * (Camera::screenWidth / 240.0f);

        maxSpeed = (Camera::screenWidth / 10.0f) * ui::maxSpeedScale;

        visionRadius = (Camera::screenWidth / 20.0f) * ui::visionRadiusScale;
        visionAngleCos = glm::cos(glm::radians(ui::visionAngleDegrees) / 2.0f);

        // Recompute vision cone vertices
        const size_t numSegments{ (ui::visionConeVertices.size() - 4) / 2 };
        const GLfloat stepSize{ glm::radians(ui::visionAngleDegrees / static_cast<float>(numSegments)) };
        const GLfloat startAngle{ glm::radians(((360.0f - ui::visionAngleDegrees) / 2.0f) - 90.0f) };

        size_t index{ 2 };
        for (size_t i{ 0 }; i <= numSegments; ++i)
        {
            const GLfloat x{glm::cos(startAngle + (stepSize * static_cast<float>(i))) * visionRadius};
            const GLfloat y{glm::sin(startAngle + (stepSize * static_cast<float>(i))) * visionRadius};
            ui::visionConeVertices[index++] = x;
            ui::visionConeVertices[index++] = y;
        }

        glBindBuffer(GL_ARRAY_BUFFER, ui::visionConeVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ui::visionConeVertices), ui::visionConeVertices.data());
    }

    inline void init()
    {
        triangleWidth = Camera::screenWidth / 220.0f;
        triangleHeight = Camera::screenHeight / 80.0f;
        glPointSize(triangleWidth / 1.5f); // The points will show up at m_pos when we render the cones so we can see exactly where the boids are visible
        recomputeStaticParams();

        // Boid triangle VAO
        // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
        const GLfloat vertices[]
        {
            -triangleWidth, -triangleHeight,   
            triangleWidth, -triangleHeight,    
            0.0f, triangleHeight                      
        };

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

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
}
