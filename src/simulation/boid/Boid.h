#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

namespace simulation::boid
{
    class Boid
    {
        public:
            static void init();
            static void showImGuiControls();
            static void updateBoids(float deltaTime);
            static void createBoid(glm::vec2 pos);
            static void renderAllBoids();

            Boid(glm::vec2 pos);

        private:
            glm::vec2 m_pos{ 0.0f };
            glm::vec2 m_velocity{ 0.0f };
            float m_hue{ 0.0f };

            static std::vector<Boid> s_boids;

            static float s_triangleWidth;
            static float s_triangleHeight;

            static float s_separation;
            static float s_alignment;
            static float s_cohesion;

            static float s_maxSpeed;

            static float s_radius;
            static float s_visionAngleCos;

            static GLuint s_VAO;

            static void recomputeStaticParams();
            static void randomizeHues();
            GLfloat getRotation() const { return -std::atan2(m_velocity.x, m_velocity.y); }
    };
}
