#include "Keyboard.hpp"
#include "Input.hpp"

#include <GLFW/glfw3.h>

void Keyboard::HandleKeys(
    GLFWwindow* window,
    const std::int32_t key,
    const std::int32_t scanCode,
    const std::int32_t action,
    const std::int32_t modifier)
{
    const auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->HandleKeyPressed(key, action);
}

bool Keyboard::IsKeyDown(const std::int32_t key) const
{
    return _keysDown.count(key) != 0;
}

bool Keyboard::IsKeyPressed(const std::int32_t key) const
{
    return _keysPressed.count(key) != 0;
}

bool Keyboard::IsKeyUp(const std::int32_t key) const
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

void Keyboard::HandleKey(std::int32_t key, std::int32_t action)
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
