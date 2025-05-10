#pragma once

#include "BoidObject.h"
#include "../../Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <random>

namespace simulation::boid::globalVars
{
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

    void init();
    void recomputeVisionConeVBO();
    void randomizeHues();

    namespace rd
    {
        extern std::random_device rd;
        extern std::mt19937 randomNumberGenerator;
        extern std::uniform_real_distribution<float> centeredDistribution;
    }
}
