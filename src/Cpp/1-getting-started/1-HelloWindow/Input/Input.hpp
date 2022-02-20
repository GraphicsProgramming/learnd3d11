#pragma once

#include "Keyboard.hpp"
#include "Mouse.hpp"

#include <cstdint>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

class Input
{
public:
    Input(GLFWwindow* window);
    ~Input();

    void Update(
        const float centerX,
        const float centerY);

    void HandleKeyPressed(
        const int32_t key,
        const int32_t action);

    void HandleMouseMove(
        const double x,
        const double y);

    [[nodiscard]] Keyboard& GetKeyboard();
    [[nodiscard]] Mouse& GetMouse();

private:
    GLFWwindow* _window = nullptr;
    Keyboard _keyboard;
    Mouse _mouse;

};
