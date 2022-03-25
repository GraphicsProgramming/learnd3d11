#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <set>

struct GLFWwindow;

class Mouse
{
public:
    static void HandleMouseButtonPressed(
        GLFWwindow* window,
        const std::int32_t button,
        const std::int32_t action,
        const std::int32_t modifiers);
    static void HandleMousePosition(
        GLFWwindow* window,
        const double x,
        const double y);

    Mouse();
    ~Mouse() = default;

    void HandleButton(
        const std::int32_t button,
        const std::int32_t action);
    void HandleMouseMove(
        const float x,
        const float y);
    void HideCursor();
    void ShowCursor();
    void Update(
        const float& centerX,
        const float& centerY);

    [[nodiscard]] bool IsButtonDown(std::int32_t button) const;
    [[nodiscard]] bool IsButtonPressed(std::int32_t button) const;
    [[nodiscard]] bool IsButtonUp(std::int32_t button) const;

    DirectX::XMFLOAT2 CursorPosition;
    DirectX::XMFLOAT2 DeltaPosition;

private:
    std::set<std::int32_t> _buttonsDown{};
    std::set<std::int32_t> _buttonsPressed{};
    std::set<std::int32_t> _buttonsUp{};
    bool _isCaptured;
};
