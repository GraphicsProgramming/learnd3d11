#include <GLFW/glfw3.h>

#include <iostream>

static void CenterWindow(GLFWwindow* windowHandle)
{
    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int32_t currentWidth = 0;
    int32_t currentHeight = 0;
    glfwGetFramebufferSize(windowHandle, &currentWidth, &currentHeight);
    glfwSetWindowPos(
        windowHandle,
        videoMode->width / 2 - currentWidth / 2,
        videoMode->height / 2 - currentHeight / 2);
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }
    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    const int32_t windowWidth = static_cast<int32_t>(videoMode->width * 0.9f);
    const int32_t windowHeight = static_cast<int32_t>(videoMode->height * 0.9f);
    const char* windowTitle = "LearnD3D11 - Hello GLFW Window";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    GLFWwindow* windowHandle = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    CenterWindow(windowHandle);
    while (!glfwWindowShouldClose(windowHandle))
    {
        glfwPollEvents();
    }
    glfwDestroyWindow(windowHandle);
    glfwTerminate();
    return 0;
}
