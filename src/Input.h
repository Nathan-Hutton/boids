#pragma once

void processKeyboardInputExit(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

bool processMouseInputClicking(GLFWwindow* window, double* xCursorPos, double* yCursorPos)
{
    static bool isClicking{ false };

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        isClicking = false;
        return false;
    }

    if (isClicking)
        return false;

    glfwGetCursorPos(window, xCursorPos, yCursorPos);
    isClicking = true;
    return true;
}

void resize_window(GLFWwindow* window, int width, int height)
{
    (void)window; // This just gets rid of the unused parameter warning
    glViewport(0, 0, width, height);
}

