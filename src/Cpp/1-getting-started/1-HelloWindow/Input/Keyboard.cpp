#include "Keyboard.hpp"
#include "Input.hpp"

#include <GLFW/glfw3.h>

void Keyboard::HandleKeys(
    GLFWwindow* window,
    const int32_t key,
    const int32_t scanCode,
    const int32_t action,
    const int32_t modifier)
{
    const auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->HandleKeyPressed(key, action);
}

bool Keyboard::IsKeyDown(const int32_t key) const
{
    return _keysDown.count(key) != 0;
}

bool Keyboard::IsKeyPressed(const int32_t key) const
{
    return _keysPressed.count(key) != 0;
}

bool Keyboard::IsKeyUp(const int32_t key) const
{
    return _keysUp.count(key) != 0;
}

void Keyboard::Update()
{
    _keysDown.clear();
    _keysUp.clear();
    _keysPressed.clear();
    _keysReleased.clear();
}

void Keyboard::HandleKey(
    const int32_t key,
    const int32_t action)
{
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
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
