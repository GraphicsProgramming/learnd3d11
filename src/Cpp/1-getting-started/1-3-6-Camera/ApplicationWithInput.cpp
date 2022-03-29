#include "ApplicationWithInput.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

ApplicationWithInput::ApplicationWithInput(const std::string_view title)
    : Application(title)
{
}

ApplicationWithInput::~ApplicationWithInput()
{
    ApplicationWithInput::Cleanup();
}

bool ApplicationWithInput::Initialize()
{
    Application::Initialize();
    glfwSetKeyCallback(GetWindow(), HandleKeyboard);
    glfwSetMouseButtonCallback(GetWindow(), HandleMouseButton);
    glfwSetCursorPosCallback(GetWindow(), HandleMouseMovement);
    return true;
}

void ApplicationWithInput::OnKey(
    const int32_t key,
    const int32_t action)
{
    switch (action)
    {
    case GLFW_PRESS:
        _keys[key] = true;
        break;
    case GLFW_RELEASE:
        _keys[key] = false;
    }
}

void ApplicationWithInput::Cleanup()
{
    glfwSetCursorPosCallback(GetWindow(), nullptr);
    glfwSetMouseButtonCallback(GetWindow(), nullptr);
    glfwSetKeyCallback(GetWindow(), nullptr);

    Application::Cleanup();
}

void ApplicationWithInput::Close()
{
    glfwSetWindowShouldClose(GetWindow(), GLFW_TRUE);
}

void ApplicationWithInput::Update()
{
    UpdateInput(.0f, .0f);
}

void ApplicationWithInput::HandleKeyboard(
    GLFWwindow* window,
    const int32_t key,
    const int32_t scanCode,
    const int32_t action,
    const int32_t modifier)
{
    ApplicationWithInput* application = static_cast<ApplicationWithInput*>(glfwGetWindowUserPointer(window));
    application->OnKey(key, action);
}

void ApplicationWithInput::HandleMouseButton(
    GLFWwindow* window,
    const int32_t button,
    const int32_t action,
    const int32_t modifiers)
{
    ApplicationWithInput* application = static_cast<ApplicationWithInput*>(glfwGetWindowUserPointer(window));
    application->OnMouseButton(button, action);
}

void ApplicationWithInput::HandleMouseMovement(
    GLFWwindow* window,
    const double x,
    const double y)
{
    ApplicationWithInput* application = static_cast<ApplicationWithInput*>(glfwGetWindowUserPointer(window));
    application->OnMouseMove(static_cast<float>(x), static_cast<float>(y));
}

void ApplicationWithInput::OnMouseButton(
    const int32_t button,
    const int32_t action)
{
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (action)
    {
    case GLFW_PRESS:
        _buttons[button] = true;
        break;
    case GLFW_RELEASE:
        _buttons[button] = false;
        break;
    }
}

void ApplicationWithInput::OnMouseMove(
    const float x,
    const float y)
{
    DeltaPosition = DirectX::XMFLOAT2(
        x - CursorPosition.x,
        y - CursorPosition.y);

    CursorPosition = DirectX::XMFLOAT2(x, y);
}

void ApplicationWithInput::UpdateInput(
    float centerX,
    float centerY)
{
    const auto window = glfwGetCurrentContext();

    DeltaPosition = DirectX::XMFLOAT2(0.0f, 0.0f);

    if (_isCaptured)
    {
        CursorPosition = DirectX::XMFLOAT2(centerX, centerY);
        glfwSetCursorPos(window, centerX, centerY);
    }
}

bool ApplicationWithInput::IsKeyDown(const int32_t key) const
{
    return _keys[key];
}

bool ApplicationWithInput::IsKeyPressed(const int32_t key) const
{
    return _keys[key];
}

bool ApplicationWithInput::IsKeyUp(const int32_t key) const
{
    return !_keys[key];
}

bool ApplicationWithInput::IsButtonDown(const int32_t button) const
{
    return _buttons[button];
}

bool ApplicationWithInput::IsButtonPressed(const int32_t button) const
{
    return _buttons[button];
}

bool ApplicationWithInput::IsButtonUp(const int32_t button) const
{
    return !_buttons[button];
}
