#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

namespace simulation::boid
{
    class BoidObject
    {
        public:
            static std::vector<BoidObject> s_boids;

            static void updateBoids(float deltaTime);
            static void createBoid(glm::vec2 pos);
            static void renderAllBoids();
            void setHue(float hue){ m_hue = hue; }

            BoidObject(glm::vec2 pos);

        private:
            glm::vec2 m_pos{ 0.0f };
            glm::vec2 m_velocity{ 0.0f };
            float m_hue{ 0.0f };

            static void randomizeHues();
            GLfloat getRotation() const { return -std::atan2(m_velocity.x, m_velocity.y); }
    };
}
