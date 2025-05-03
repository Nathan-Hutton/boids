#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "ShaderHandler.h"
#include "Input.h"
#include "Boid.h"
#include "Camera.h"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor { glfwGetPrimaryMonitor() };
    const GLFWvidmode* mode { glfwGetVideoMode(monitor) };
    GLFWwindow* window { glfwCreateWindow(mode->width, mode->height, "Boids", monitor, NULL) };
    if (window == NULL)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glViewport(0, 0, mode->width, mode->height);
    glfwSetFramebufferSizeCallback(window, resize_window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    Camera::init(mode->width, mode->height);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Glew initialization failed");
    }

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    const GLuint shaderProgram{ ShaderHandler::compileShader(std::vector<std::string>{"../shaders/shader.vert", "../shaders/shader.frag"}) };
    glUseProgram(shaderProgram);

    std::vector<Boid> boids;
    boids.push_back(Boid{Camera::cameraCenter});

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();

        double xCursorPos, yCursorPos;
        if (processMouseInputClicking(window, &xCursorPos, &yCursorPos))
            boids.push_back(Boid{{static_cast<float>(xCursorPos), static_cast<float>(yCursorPos), 0.0f}});

        for (const Boid& boid : boids)
        {
            const glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, boid.getPos()) };
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            boid.render();
        }

        processKeyboardInputExit(window);
        glfwSwapBuffers(window);
    }
    return 0;
}
