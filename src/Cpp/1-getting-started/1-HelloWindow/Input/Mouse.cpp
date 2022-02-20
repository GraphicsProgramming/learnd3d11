#include "Mouse.hpp"
#include "Input.hpp"

#include <GLFW/glfw3.h>

void Mouse::HandleMouseButtonPressed(
    GLFWwindow* window,
    const int32_t button,
    const int32_t action,
    const int32_t modifiers)
{
    _CRT_UNUSED(modifiers);
    const auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->HandleKeyPressed(button, action);
}

void Mouse::HandleMousePosition(
    GLFWwindow* window,
    const double x,
    const double y)
{
    const auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->HandleMouseMove(x, y);
}

Mouse::Mouse(): CursorPosition(), DeltaPosition(), _isCaptured(false)
{
}

void Mouse::HandleButton(
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

void Mouse::HandleMouseMove(
    const float x,
    const float y)
{
    DeltaPosition = DirectX::XMFLOAT2(
        x - CursorPosition.x,
        y - CursorPosition.y);

    CursorPosition = DirectX::XMFLOAT2(x, y);
}

void Mouse::HideCursor()
{
    const auto window = glfwGetCurrentContext();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _isCaptured = true;
}

bool Mouse::IsButtonDown(const int32_t button) const
{
    return _buttonsDown.count(button) != 0;
}

bool Mouse::IsButtonPressed(const int32_t button) const
{
    return _buttonsPressed.count(button) != 0;
}

bool Mouse::IsButtonUp(const int32_t button) const
{
    return _buttonsUp.count(button) != 0;
}

void Mouse::ShowCursor()
{
    const auto window = glfwGetCurrentContext();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    _isCaptured = false;
}

void Mouse::Update(
    const float& centerX,
    const float& centerY)
{
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
