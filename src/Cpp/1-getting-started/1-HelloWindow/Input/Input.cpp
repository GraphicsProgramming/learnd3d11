#include "Input.hpp"

#include <GLFW/glfw3.h>

Input::Input(GLFWwindow* window)
{
    _window = window;
    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, Keyboard::HandleKeys);
    glfwSetMouseButtonCallback(_window, Mouse::HandleMouseButtonPressed);
    glfwSetCursorPosCallback(_window, Mouse::HandleMousePosition);
}

Input::~Input()
{
    glfwSetCursorPosCallback(_window, nullptr);
    glfwSetMouseButtonCallback(_window, nullptr);
    glfwSetKeyCallback(_window, nullptr);
    glfwSetWindowUserPointer(_window, nullptr);
}

Keyboard& Input::GetKeyboard()
{
    return _keyboard;
}

Mouse& Input::GetMouse()
{
    return _mouse;
}

void Input::HandleKeyPressed(
    const int32_t key,
    const int32_t action)
{
    _keyboard.HandleKey(key, action);
    _mouse.HandleButton(key, action);
}

void Input::HandleMouseMove(
    const double x,
    const double y)
{
    _mouse.HandleMouseMove(
        static_cast<float>(x),
        static_cast<float>(y));
}

void Input::Update(
    const float centerX,
    const float centerY)
{
    _keyboard.Update();
    _mouse.Update(centerX, centerY);
}
