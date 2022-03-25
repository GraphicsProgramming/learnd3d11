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

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
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
    const int32_t windowTop = videoMode->height / 2 - _height / 2;
    glfwSetWindowPos(_window, windowLeft, windowTop);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, HandleResize);
    glfwSetKeyCallback(_window, HandleKeyboard);
    glfwSetMouseButtonCallback(_window, HandleMouseButton);
    glfwSetCursorPosCallback(_window, HandleMouseMovement);
    return true;
}

void Application::OnResize(
    const int32_t width,
    const int32_t height)
{
    _width = width;
    _height = height;
}

void Application::OnKey(
    const int32_t key,
    const int32_t action)
{
    switch (action)
    {
    case GLFW_PRESS:
        _keysPressed.insert(key);
        _keysDown.insert(key);
        break;
    case GLFW_RELEASE:
        _keysReleased.insert(key);
        _keysDown.erase(key);
    }
}

void Application::Cleanup()
{
    glfwSetCursorPosCallback(_window, nullptr);
    glfwSetMouseButtonCallback(_window, nullptr);
    glfwSetKeyCallback(_window, nullptr);
    glfwSetFramebufferSizeCallback(_window, nullptr);
    glfwSetWindowUserPointer(_window, nullptr);

    glfwDestroyWindow(_window);
    glfwTerminate();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Application::Close()
{
    glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

void Application::Run()
{
    if (!Initialize())
    {
        return;
    }

    if (!Load())
    {
        return;
    }

    while (!glfwWindowShouldClose(_window))
    {
        UpdateInput(
            static_cast<float>(_width) / 2.0f,
            static_cast<float>(_height) / 2.0f);

        glfwPollEvents();
        Update();
        Render();
    }
}

void Application::HandleResize(
    GLFWwindow* window,
    const int32_t width,
    const int32_t height)
{
    Application& application = *static_cast<Application*>(glfwGetWindowUserPointer(window));
    application.OnResize(width, height);
}

void Application::HandleKeyboard(
    GLFWwindow* window,
    const int32_t key,
    const int32_t scanCode,
    const int32_t action,
    const int32_t modifier)
{
    Application& application = *static_cast<Application*>(glfwGetWindowUserPointer(window));
    application.OnKey(key, action);
}

void Application::HandleMouseButton(
    GLFWwindow* window,
    const int32_t button,
    const int32_t action,
    const int32_t modifiers)
{
    Application& application = *static_cast<Application*>(glfwGetWindowUserPointer(window));
    application.OnMouseButton(button, action);
}

void Application::HandleMouseMovement(
    GLFWwindow* window,
    const double x,
    const double y)
{
    Application& application = *static_cast<Application*>(glfwGetWindowUserPointer(window));
    application.OnMouseMove(static_cast<float>(x), static_cast<float>(y));
}

void Application::OnMouseButton(
    const int32_t button,
    const int32_t action)
{
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (action)
    {
    case GLFW_PRESS:
        _buttonsPressed.insert(button);
        _buttonsDown.insert(button);
        break;
    case GLFW_RELEASE:
        _buttonsUp.insert(button);
        _buttonsDown.erase(button);
        break;
    }
}

void Application::OnMouseMove(
    const float x,
    const float y)
{
    DeltaPosition = DirectX::XMFLOAT2(
        x - CursorPosition.x,
        y - CursorPosition.y);

    CursorPosition = DirectX::XMFLOAT2(x, y);
}

void Application::UpdateInput(
    float centerX,
    float centerY)
{
    _keysDown.clear();
    _keysUp.clear();
    _keysPressed.clear();
    _keysReleased.clear();

    const auto window = glfwGetCurrentContext();
    _buttonsDown.clear();
    _buttonsPressed.clear();
    _buttonsUp.clear();

    DeltaPosition = DirectX::XMFLOAT2(0.0f, 0.0f);

    if (_isCaptured)
    {
        CursorPosition = DirectX::XMFLOAT2(centerX, centerY);
        glfwSetCursorPos(window, centerX, centerY);
    }
}

GLFWwindow* Application::GetWindow() const
{
    return _window;
}

int32_t Application::GetWindowWidth() const
{
    return _width;
}

int32_t Application::GetWindowHeight() const
{
    return _height;
}

bool Application::IsKeyDown(const int32_t key) const
{
    return _keysDown.count(key) != 0;
}

bool Application::IsKeyPressed(const int32_t key) const
{
    return _keysPressed.count(key) != 0;
}

bool Application::IsKeyUp(const int32_t key) const
{
    return _keysUp.count(key) != 0;
}

bool Application::IsButtonDown(const int32_t button) const
{
    return _buttonsDown.count(button) != 0;
}

bool Application::IsButtonPressed(const int32_t button) const
{
    return _buttonsPressed.count(button) != 0;
}

bool Application::IsButtonUp(const int32_t button) const
{
    return _buttonsUp.count(button) != 0;
}
