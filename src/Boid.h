#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Camera.h"

class Boid
{
    public:
        Boid(glm::vec2 pos)
        {
            m_pos = pos;

            // The coordinate frame is using screen resolution where the top left is 0,0. X points right and Y points down (because this is what GLFW uses)
            const float triangleWidth{ Camera::width / 60.0f };
            const float triangleHeight{ Camera::height / 60.0f };
            const GLfloat vertices[]
            {
                -triangleWidth / 2.0f, -triangleHeight,   // Bottom left
                triangleWidth / 2.0f, -triangleHeight,    // Bottom right
                0.0f, triangleHeight                      // Middle top
            };

            glGenVertexArrays(1, &m_VAO);
            glBindVertexArray(m_VAO);

            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        glm::vec2 getPos() const { return m_pos; }
        void setPos(glm::vec2 pos) { m_pos = pos; }

        void render() const
        {
            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

    private:
        GLfloat m_rotation{ 0.0f };
        glm::vec2 m_pos{ 0.0f };
        GLuint m_VAO;
};
