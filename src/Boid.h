#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <iostream>

class Boid
{
    public:
        Boid()
        {
            constexpr float vertices[]
            {
                -0.5f, -0.5f, 0.0f,     // Bottom left
                0.5f, -0.5f, 0.0f,      // Bottom right
                0.0f, 0.5f, 0.0f        // Middle top
            };

            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        }

        void render()
        {
            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        }

    private:
        GLfloat m_rotation{};
        glm::vec3 m_pos{};
        GLuint m_VAO;
};
