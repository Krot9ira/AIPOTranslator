// All rights reserved by Daniil Grigoriev.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include "POTranlator.h"
#include <curl/curl.h>
#include <Shlobj.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Utility.h"
#include "UI/ImGuiManager.h"

int main()
{

    curl_global_init(CURL_GLOBAL_ALL);

    glfwInit();

    // Set OpenGL version for modern context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 640, "AIPO Translator", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        curl_global_cleanup();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGL())
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        curl_global_cleanup();
        return -1;
    }

    ImGuiManager::Instance().Initialize();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // MainLoop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Begginig of frame ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render main window
        ImGuiManager::Instance().RenderMainWindow();

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGuiManager::Instance().Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    curl_global_cleanup();

    return 0;
}
