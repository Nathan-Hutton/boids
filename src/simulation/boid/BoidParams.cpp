#include "BoidParams.h"
#include "../UI.h"

#include <random>

namespace
{
    std::random_device rd;
    std::mt19937 randomNumberGenerator{rd()};
    std::uniform_real_distribution<float> centeredDistribution{-1.0f, 1.0f};
}

namespace simulation::boid
{
    std::vector<Boid> boids{};
    GLuint VAO{};
    float triangleWidth{};
    float triangleHeight{};

    float defaultSeparation{};
    float separation{};
    float defaultAlignment{};
    float alignment{};
    float defaultCohesion{};
    float cohesion{};

    float defaultMaxSpeed{};
    float maxSpeed{};

    float defaultVisionRadius{};
    float visionRadius{};
    float visionAngleCos{};

    float saturation{ 90.0f / 255.0f };
    float brightness{ 200.0f / 255.0f };

    void recomputeVisionConeVBO()
    {
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

    void init()
    {
        triangleWidth = Camera::screenWidth / 220.0f;
        triangleHeight = Camera::screenHeight / 80.0f;
        glPointSize(triangleWidth / 1.5f); // The points will show up at m_pos when we render the cones so we can see exactly where the boids are visible

        defaultSeparation = (Camera::screenWidth * 0.15f);
        separation = defaultSeparation;
        defaultAlignment = (Camera::screenWidth * 0.15f);
        alignment = defaultAlignment;
        defaultCohesion = (Camera::screenWidth / 240.0f);
        cohesion = defaultCohesion;

        defaultMaxSpeed = (Camera::screenWidth / 10.0f);
        maxSpeed = defaultMaxSpeed;

        defaultVisionRadius = (Camera::screenWidth / 20.0f);
        visionRadius = defaultVisionRadius;
        visionAngleCos = glm::cos(glm::radians(ui::visionAngleDegrees) / 2.0f);

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

        recomputeVisionConeVBO();
    }

    void randomizeHues()
    {
        for (Boid& boid : boids)
            boid.setHue((centeredDistribution(randomNumberGenerator) + 1.0f) / 2.0f);
    }
}
