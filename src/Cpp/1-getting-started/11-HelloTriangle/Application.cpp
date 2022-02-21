#include "Application.hpp"
#include "Graphics/D3DContext.hpp"
#include "Input/Input.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

Application::Application(const std::string_view title)
    : _input(nullptr), _window(nullptr), _width(0), _height(0), _title(title)
{
}

Application::~Application()
{
    Application::Cleanup();
}

void Application::Run()
{
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

    const auto primaryMonitor = glfwGetPrimaryMonitor();
    const auto videoMode = glfwGetVideoMode(primaryMonitor);
    _width = static_cast<std::int32_t>(videoMode->width * 0.9f);
    _height = static_cast<std::int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
    if (_window == nullptr)
    {
        Cleanup();
        return false;
    }
    _input = std::make_unique<Input>(_window);

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

HWND Application::GetWindowHandle() const
{
    return glfwGetWin32Window(_window);
}

int32_t Application::GetWindowWidth() const
{
    return _width;
}

int32_t Application::GetWindowHeight() const
{
    return _height;
}

bool Application::IsButtonPressed(const std::int32_t button) const
{
    return _input->GetMouse().IsButtonPressed(button);
}
