#include "Application.hpp"
#include "Input/Input.hpp"

#include <GLFW/glfw3.h>

Application::Application(
    const std::int32_t width,
    const std::int32_t height,
    const std::string_view title)
    : _window(nullptr), _width(width), _height(height), _title(title), _input(nullptr)
{
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
        glfwSwapBuffers(_window);
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

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
    if (_window == nullptr)
    {
        Cleanup();
        return false;
    }
    _input = std::make_unique<Input>(_window);

    const auto primaryMonitor = glfwGetPrimaryMonitor();
    const auto videoMode = glfwGetVideoMode(primaryMonitor);
    const auto windowLeft = videoMode->width / 2 - _width / 2;
    const auto windowTop = videoMode->height / 2 - _height / 2;
    glfwSetWindowPos(_window, windowLeft, windowTop);

    return true;
}

bool Application::IsKeyDown(const std::int32_t key) const
{
    return _input->GetKeyboard().IsKeyDown(key);
}

bool Application::IsKeyPressed(const std::int32_t key) const
{
    return _input->GetKeyboard().IsKeyPressed(key);
}

bool Application::IsKeyUp(const std::int32_t key) const
{
    return _input->GetKeyboard().IsKeyUp(key);
}

bool Application::IsButtonPressed(const std::int32_t button) const
{
    return _input->GetMouse().IsButtonPressed(button);
}
