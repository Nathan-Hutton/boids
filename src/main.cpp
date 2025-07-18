#include "ShaderHandler.h"
#include "Input.h"
#include "simulation/boid/BoidObject.h"
#include "simulation/boid/BoidParams.h"
#include "simulation/obstacle/Obstacle.h"
#include "simulation/UI.h"
#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

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
    Camera::init(window, static_cast<float>(mode->width), static_cast<float>(mode->height));

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Glew initialization failed");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    ImGui::GetIO().IniFilename = nullptr;

    simulation::boid::globalVars::init();
    simulation::obstacle::init();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ShaderHandler::shaderProgram = ShaderHandler::compileShader(std::vector<std::string>{"../shaders/shader.vert", "../shaders/shader.frag"});
    glUseProgram(ShaderHandler::shaderProgram);

    bool showSettingsUI{ false };
    float lastUpdateTime{ static_cast<float>(glfwGetTime()) };

	// This is something I only needed to add after switching to Hyprland. The DPI scaling screws the mouse cursor
	// pos up with glfwGetCursorPos(). This block calculates that scale factor so we can correct it.
	double cursorScaleFactor{ 1.0 };
	{
		int winW, winH;
		glfwGetWindowSize(window, &winW, &winH);
		if (winW != Camera::screenWidth)
		{
			// For some reason, a few frames need to be rendered before glfw will give us what it thinks is the correct
			// window size. This loop renders a few bogus frames to make this happen
			const int oldWinW{ winW };
			while (winW == oldWinW)
			{
				glfwPollEvents();
				glfwGetWindowSize(window, &winW, &winH);
				glfwSwapBuffers(window);
			}

			cursorScaleFactor = Camera::screenWidth / winW;
		}
	}

    simulation::ui::cursors::initCursors(cursorScaleFactor);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io{ ImGui::GetIO() };

        if (processPressingSpace(window))
            simulation::ui::placingBoids = !simulation::ui::placingBoids;

        if (processPressingAlt(window))
        {
            simulation::ui::avoidMouse = !simulation::ui::avoidMouse;
            if (simulation::ui::avoidMouse)
                glfwSetCursor(window, simulation::ui::cursors::redCursor);
            else
                glfwSetCursor(window, simulation::ui::cursors::whiteCursor);
        }

        double xCursorPos, yCursorPos;
        glfwGetCursorPos(window, &xCursorPos, &yCursorPos);
		xCursorPos *= simulation::ui::cursors::cursorScaleFactor;
		yCursorPos *= simulation::ui::cursors::cursorScaleFactor;
        if (!io.WantCaptureMouse && processMouseInputClicking(window))
        {
            if (simulation::ui::placingBoids)
                simulation::boid::BoidObject::createBoid({xCursorPos, yCursorPos});
            else
                simulation::obstacle::Obstacle::createObstacle({xCursorPos, yCursorPos});
        }

        // Handle boids
        const float currentTime{ static_cast<float>(glfwGetTime()) };
        const float deltaTime{ currentTime - lastUpdateTime };
        lastUpdateTime = currentTime;
        simulation::boid::BoidObject::updateBoids(deltaTime);
        simulation::boid::BoidObject::renderAllBoids();
        simulation::obstacle::Obstacle::renderAllObstacles();

        // Show a translucent version of an obstacle where the cursor is if we're in the placing obstacles mode
        if (!simulation::ui::placingBoids)
        {
            glBindVertexArray(simulation::obstacle::VAO);
            glUniform3fv(glGetUniformLocation(ShaderHandler::shaderProgram, "color"), 1, glm::value_ptr(glm::vec3{ 0.5f, 0.0f, 0.0f }));

            const glm::mat4 model{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ xCursorPos, yCursorPos, 0.0f }) };
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandler::shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(Camera::viewProjection * model));
            glDrawArrays(GL_TRIANGLE_FAN, 0, simulation::obstacle::vertices.size());
        }

        if (processPressingF1Key(window))
            showSettingsUI = !showSettingsUI;

        if (showSettingsUI)
            simulation::ui::renderUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        processKeyboardInputExit(window);
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
