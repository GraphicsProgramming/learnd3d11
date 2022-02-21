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
    const int32_t windowWidth = 1280;
    const int32_t windowHeight = 720;
    const char* windowTitle = "LearnD3D11 - Hello GLFW Window";
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    GLFWwindow* windowHandle = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    CenterWindow(windowHandle);
    while (!glfwWindowShouldClose(windowHandle))
    {
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
