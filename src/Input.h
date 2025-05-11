#pragma once

#include <GLFW/glfw3.h>

void processKeyboardInputExit(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

bool processMouseInputClicking(GLFWwindow* window)
{
    static bool isClicking{ false };

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        isClicking = false;
        return false;
    }

    if (isClicking)
        return false;

    isClicking = true;
    return true;
}

bool processPressingF1Key(GLFWwindow* window)
{
    static bool f1KeyPressed{ false };

    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        if (f1KeyPressed)
            return false;

        f1KeyPressed = true;
        return true;
    }

    f1KeyPressed = false;
    return false;
}

bool processPressingSpace(GLFWwindow* window)
{
    static bool spaceKeyPressed{ false };

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (spaceKeyPressed)
            return false;

        spaceKeyPressed = true;
        return true;
    }

    spaceKeyPressed = false;
    return false;
}

void resize_window(GLFWwindow* window, int width, int height)
{
    (void)window; // This just gets rid of the unused parameter warning
    glViewport(0, 0, width, height);
}

