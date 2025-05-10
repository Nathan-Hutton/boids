#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>

namespace simulation::obstacle
{
    class Obstacle
    {
        public:
            static std::vector<Obstacle> s_obstacles;

            static void createObstacle(glm::vec2 pos);
            static void renderAllObstacles();

            Obstacle(glm::vec2 pos) : m_pos{ pos } {}

        private:
            glm::vec2 m_pos{ 0.0f };
    };

    extern float radius;
    extern GLuint VAO, VBO;
    extern std::array<GLfloat, 104> vertices; // So really 51 vertices (including middle)

    void init();
    void recomputeVBO();
    void makeBigCircleObstacle();
}
