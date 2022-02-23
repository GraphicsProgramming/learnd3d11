#include "Application.hpp"

#include <GLFW/glfw3.h>

Application::Application(const std::string_view title)
    : _title(title)
{
}

Application::~Application()
{
    Application::Cleanup();
}

bool Application::Initialize()
{
    if (!glfwInit())
    {
        return false;
    }

    GLFWmonitor*       primaryMonitor  = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode       = glfwGetVideoMode(primaryMonitor);
    _width  = static_cast<int32_t>(videoMode->width * 0.9f);
    _height = static_cast<int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(
        _width,
        _height,
        _title.data(),
        nullptr,
        nullptr);
    if (_window == nullptr)
    {
        Cleanup();
        return false;
    }
    const int32_t windowLeft = videoMode->width / 2 - _width / 2;
    const int32_t windowTop  = videoMode->height / 2 - _height / 2;
    glfwSetWindowPos(_window, windowLeft, windowTop);
    return true;
}

void Application::Cleanup()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}


void Application::Run()
{
    if (!Initialize())
    {
        return;
    }

    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
        Update();
        Render();
    }
}

GLFWwindow* Application::GetWindow() const
{
    return _window;
}

void Application::Close()
{
    glfwSetWindowShouldClose(_window, true);
}
