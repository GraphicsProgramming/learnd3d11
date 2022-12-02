#pragma once

#include <DirectXMath.h>

#include <array>
#include <cstdint>
#include <memory>

#include <Application.hpp>

struct GLFWwindow;

class ApplicationWithInput : public Application
{
public:
    ApplicationWithInput(const std::string& title);
    virtual ~ApplicationWithInput();

protected:
    void OnKey(
        int32_t key,
        int32_t action);
    void OnMouseButton(
        int32_t button,
        int32_t action);
    void OnMouseMove(
        float x,
        float y);

    virtual bool Initialize() override;
    virtual void Cleanup() override;
    void Close();
    virtual void Update() override;

    [[nodiscard]] bool IsButtonPressed(int32_t button) const;
    [[nodiscard]] bool IsButtonDown(int32_t button) const;
    [[nodiscard]] bool IsButtonUp(int32_t button) const;
    [[nodiscard]] bool IsKeyDown(int32_t key) const;
    [[nodiscard]] bool IsKeyPressed(int32_t key) const;
    [[nodiscard]] bool IsKeyUp(int32_t key) const;

    DirectX::XMFLOAT2 DeltaPosition = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 CursorPosition = { 0.0f, 0.0f };

private:
    static void HandleResize(
        GLFWwindow* window,
        int32_t width,
        int32_t height);

    static void HandleKeyboard(
        GLFWwindow* window,
        int32_t key,
        int32_t scanCode,
        int32_t action,
        int32_t modifier);
    static void HandleMouseButton(
        GLFWwindow* window,
        int32_t button,
        int32_t action,
        int32_t modifiers);
    static void HandleMouseMovement(
        GLFWwindow* window,
        double x,
        double y);

    void UpdateInput(
        float centerX,
        float centerY);

    std::array<bool, 512> _keys{};
    std::array<bool, 512> _buttons{};
    bool _isCaptured = false;
};
