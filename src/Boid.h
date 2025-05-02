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
            constexpr GLfloat vertices[]
            {
                -0.2f, -0.5f, 0.0f,    // Bottom left
                0.2f, -0.5f, 0.0f,     // Bottom right
                0.0f, 0.5f, 0.0f        // Middle top
            };

            glGenVertexArrays(1, &m_VAO);
            glBindVertexArray(m_VAO);

            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        void render()
        {
            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

    private:
        GLfloat m_rotation{};
        glm::vec3 m_pos{};
        GLuint m_VAO;
};
