#include "Obstacle.h"
#include "../../Camera.h"
#include "../../ShaderHandler.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <iostream>

namespace simulation::obstacle
{
    std::vector<Obstacle> Obstacle::s_obstacles{};
    std::array<GLfloat, 104> vertices{};
    float defaultRadius{};
    float radius{};
    GLuint VAO{};
    GLuint VBO{};
}

void simulation::obstacle::init()
{
    defaultRadius = Camera::screenWidth / 100.0f;
    radius = defaultRadius;

    // Boid vision cone/circle VAO. This is already initialized to zero
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    recomputeVBO();
}

void simulation::obstacle::recomputeVBO()
{
    // Recompute vision cone vertices
    constexpr size_t numSegments{ (vertices.size() - 4) / 2 };
    constexpr GLfloat stepSize{ glm::two_pi<float>() / static_cast<float>(numSegments) };

    size_t index{ 2 };
    for (size_t i{ 0 }; i <= numSegments; ++i)
    {
        const GLfloat x{glm::cos((stepSize * static_cast<float>(i))) * radius};
        const GLfloat y{glm::sin((stepSize * static_cast<float>(i))) * radius};
        vertices[index++] = x;
        vertices[index++] = y;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices.data());
}

void simulation::obstacle::Obstacle::createObstacle(glm::vec2 pos)
{
    s_obstacles.emplace_back(pos);
}

void simulation::obstacle::Obstacle::renderAllObstacles()
{
    glBindVertexArray(VAO);
    glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{1.0f, 0.0f, 0.0f}));

    for (const Obstacle& obstacle : s_obstacles)
    {
        glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ obstacle.m_pos, 0.0f }) };
        glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
    }
}
