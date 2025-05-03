#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

class Boid
{
    public:
        static std::vector<Boid> boids;

        Boid(glm::vec2 pos);
        void render() const;

        glm::vec2 getPos() const { return m_pos; }
        GLfloat getRotation() const { return m_rotation; }
        void setPos(glm::vec2 pos) { m_pos = pos; }
        void setRotation(GLfloat rotation) { m_rotation = rotation; }

    private:
        GLfloat m_rotation{ 0.0f };
        glm::vec2 m_pos{ 0.0f };
        GLuint m_VAO;
};
