#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

class Boid
{
    public:
        static std::vector<Boid> s_boids;
        static void init(float screenWidth, float screenHeight);
        static void updateBoids(float deltaTime);
        static void createBoid(glm::vec2 pos);

        Boid(glm::vec2 pos);
        void render() const;

        glm::vec2 getPos() const { return m_pos; }
        GLfloat getRotation() const { return -std::atan2(m_velocity.x, m_velocity.y); }
        void setPos(glm::vec2 pos) { m_pos = pos; }

    private:
        glm::vec2 m_pos{ 0.0f };
        glm::vec2 m_velocity{ 0.0f };
        GLuint m_VAO;

        static float s_maxSpeed;
        static float s_triangleWidth;
        static float s_triangleHeight;
        static float s_radius;
        static float s_visionAngleCos;
};
