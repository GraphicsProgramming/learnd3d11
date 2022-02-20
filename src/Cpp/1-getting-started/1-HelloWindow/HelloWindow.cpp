#include "HelloWindow.hpp"
#include "Input/Input.hpp"

#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

HelloWindowApplication::HelloWindowApplication(
    const std::int32_t width,
    const std::int32_t height,
    const std::string_view title)
        : Application(width, height, title)
{
}

void HelloWindowApplication::Render()
{
}

void HelloWindowApplication::Update()
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        std::cout << "Key: Escape\n";
        Close();
    }

    if (IsButtonPressed(GLFW_MOUSE_BUTTON_1))
    {
        const auto cursorPosition = _input->GetMouse().CursorPosition;
        std::cout << std::to_string(cursorPosition.x) << ", " << std::to_string(cursorPosition.y) << "\n";
    }
}
