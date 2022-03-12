#include "Application.hpp"
#include "Input/Input.hpp"

#include <GLFW/glfw3.h>

Application::Application(const std::string_view title)
{
    _title = title;
}

Application::~Application()
{
    Application::Cleanup();
}

void Application::Run()
{
    if (!Initialize())
    {
        return;
    }

    while (!glfwWindowShouldClose(_window))
    {
        _input->Update(
            static_cast<float>(_width) / 2.0f,
            static_cast<float>(_height) / 2.0f);
        glfwPollEvents();
        Update();
        Render();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Application::Cleanup()
{
    glfwTerminate();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Application::Close()
{
    glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

bool Application::Initialize()
{
    if (!glfwInit())
    {
        return false;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    _width = static_cast<int32_t>(videoMode->width * 0.9f);
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
    _input = std::make_unique<Input>(_window);

    const int32_t windowLeft = videoMode->width / 2 - _width / 2;
    const int32_t windowTop = videoMode->height / 2 - _height / 2;
    glfwSetWindowPos(
        _window,
        windowLeft,
        windowTop);

    return true;
}

bool Application::IsKeyDown(const int32_t key) const
{
    return _input->GetKeyboard().IsKeyDown(key);
}

bool Application::IsKeyPressed(const int32_t key) const
{
    return _input->GetKeyboard().IsKeyPressed(key);
}

bool Application::IsKeyUp(const int32_t key) const
{
    return _input->GetKeyboard().IsKeyUp(key);
}

bool Application::IsButtonPressed(const int32_t button) const
{
    return _input->GetMouse().IsButtonPressed(button);
}
