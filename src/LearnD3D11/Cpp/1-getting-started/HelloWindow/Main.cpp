#include <GLFW/glfw3.h>

void CenterWindow(GLFWwindow* window)
{
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    const auto primaryMonitor = glfwGetPrimaryMonitor();
    const auto videoMode = glfwGetVideoMode(primaryMonitor);
    const auto windowLeft = videoMode->width / 2 - windowWidth / 2;
    const auto windowTop = videoMode->height / 2 - windowHeight / 2;
    glfwSetWindowPos(window, windowLeft, windowTop);
}

int main(int argc, char* argv[])
{
    if (!glfwInit())
    {
        return -1;
    }

    const auto window = glfwCreateWindow(1920, 1080, "LearnD3D11 - Hello Window", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return -1;
    }

    CenterWindow(window);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}