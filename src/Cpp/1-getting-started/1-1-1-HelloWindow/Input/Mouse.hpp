#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <set>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

class Mouse
{
public:
    static void HandleMouseButtonPressed(
        GLFWwindow* window,
        const int32_t button,
        const int32_t action,
        const int32_t modifiers);

    static void HandleMousePosition(
        GLFWwindow* window,
        const double x,
        const double y);

    Mouse();
    ~Mouse() = default;

    void HandleButton(
        const int32_t button,
        const int32_t action);

    void HandleMouseMove(
        const float x,
        const float y);

    void HideCursor();

    void ShowCursor();

    void Update(
        const float& centerX,
        const float& centerY);

    [[nodiscard]] bool IsButtonDown(int32_t button) const;
    [[nodiscard]] bool IsButtonPressed(int32_t button) const;
    [[nodiscard]] bool IsButtonUp(int32_t button) const;

    DirectX::XMFLOAT2 CursorPosition;
    DirectX::XMFLOAT2 DeltaPosition;

private:
    std::set<int32_t> _buttonsDown{};
    std::set<int32_t> _buttonsPressed{};
    std::set<int32_t> _buttonsUp{};
    bool _isCaptured;
};
