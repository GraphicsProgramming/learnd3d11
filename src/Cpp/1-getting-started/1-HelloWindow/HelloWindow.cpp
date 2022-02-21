#include "HelloWindow.hpp"
#include "Input/Input.hpp"

#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

HelloTriangleApplication::HelloTriangleApplication(const std::string_view title)
    : Application(title)
{
}

void HelloTriangleApplication::Render()
{

}

void HelloTriangleApplication::Update()
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
