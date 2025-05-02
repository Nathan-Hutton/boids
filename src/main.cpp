#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "ShaderHandler.h"
#include "Input.h"
#include "Boid.h"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor { glfwGetPrimaryMonitor() };
    const GLFWvidmode* mode { glfwGetVideoMode(monitor) };
    GLFWwindow* window { glfwCreateWindow(mode->width, mode->height, "Rigid-body", monitor, NULL) };
    if (window == NULL)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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

    const GLuint shaderProgram{ ShaderHandler::compileShader(std::vector<std::string>{"../shaders/shader.vert", "../shaders/shader.frag"}) };
    glUseProgram(shaderProgram);

    Boid boid{};

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processKeyboardInputExit(window);
        glfwSwapBuffers(window);
    }
    return 0;
}
