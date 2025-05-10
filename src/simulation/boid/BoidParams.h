#pragma once

#include "Boid.h"
#include "../../Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

namespace simulation::boid
{
    extern std::vector<Boid> boids;
    extern GLuint VAO;
    extern float triangleWidth;
    extern float triangleHeight;

    extern float defaultSeparation;
    extern float separation;
    extern float defaultAlignment;
    extern float alignment;
    extern float defaultCohesion;
    extern float cohesion;

    extern float defaultMaxSpeed;
    extern float maxSpeed;

    extern float defaultVisionRadius;
    extern float visionRadius;
    extern float visionAngleCos;

    extern float saturation;
    extern float brightness;

    void recomputeVisionConeVBO();
    void init();
    void randomizeHues();
}
