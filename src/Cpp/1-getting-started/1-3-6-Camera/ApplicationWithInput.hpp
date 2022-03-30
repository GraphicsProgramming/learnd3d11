#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <memory>
#include <array>

#include <Application.hpp>

struct GLFWwindow;

class ApplicationWithInput : public Application
{
public:
    ApplicationWithInput(const std::string& title);
    virtual ~ApplicationWithInput();

protected:
    void OnKey(
        const int32_t key,
        const int32_t action);
    void OnMouseButton(
        const int32_t button,
        const int32_t action);
    void OnMouseMove(
        const float x,
        const float y);

    virtual bool Initialize() override;
    virtual void Cleanup() override;
    void Close();
    virtual void Update() override;

    [[nodiscard]] bool IsButtonPressed(const int32_t button) const;
    [[nodiscard]] bool IsButtonDown(const int32_t button) const;
    [[nodiscard]] bool IsButtonUp(const int32_t button) const;
    [[nodiscard]] bool IsKeyDown(const int32_t key) const;
    [[nodiscard]] bool IsKeyPressed(const int32_t key) const;
    [[nodiscard]] bool IsKeyUp(const int32_t key) const;

    DirectX::XMFLOAT2 DeltaPosition = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 CursorPosition = { 0.0f, 0.0f };

private:
    static void HandleResize(
        GLFWwindow* window,
        const int32_t width,
        const int32_t height);

    static void HandleKeyboard(
        GLFWwindow* window,
        const int32_t key,
        const int32_t scanCode,
        const int32_t action,
        const int32_t modifier);
    static void HandleMouseButton(
        GLFWwindow* window,
        const int32_t button,
        const int32_t action,
        const int32_t modifiers);
    static void HandleMouseMovement(
        GLFWwindow* window,
        const double x,
        const double y);

    void UpdateInput(
        float centerX,
        float centerY);

    std::array<bool, 512> _keys{};
    std::array<bool, 512> _buttons{};
    bool _isCaptured = false;
};
