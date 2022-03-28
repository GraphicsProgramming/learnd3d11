#pragma once

#include <glm/vec2.hpp>

#include <string_view>
#include <cstdint>
#include <memory>
#include <array>

struct GLFWwindow;

class Application
{
public:
    Application(const std::string_view title);
    virtual ~Application();

    void Run();
protected:
    virtual void OnResize(
        const int32_t width,
        const int32_t height);
    void OnKey(
        const int32_t key,
        const int32_t action);
    void OnMouseButton(
        const int32_t button,
        const int32_t action);
    void OnMouseMove(
        const float x,
        const float y);

    virtual bool Initialize();
    virtual bool Load() = 0;
    virtual void Cleanup();
    void Close();
    virtual void Render() = 0;
    virtual void Update() = 0;

    [[nodiscard]] GLFWwindow* GetWindow() const;
    [[nodiscard]] int32_t GetWindowWidth() const;
    [[nodiscard]] int32_t GetWindowHeight() const;

    [[nodiscard]] bool IsButtonPressed(const int32_t button) const;
    [[nodiscard]] bool IsButtonDown(const int32_t button) const;
    [[nodiscard]] bool IsButtonUp(const int32_t button) const;
    [[nodiscard]] bool IsKeyDown(const int32_t key) const;
    [[nodiscard]] bool IsKeyPressed(const int32_t key) const;
    [[nodiscard]] bool IsKeyUp(const int32_t key) const;

    glm::vec2 DeltaPosition = { 0.0f, 0.0f };
    glm::vec2 CursorPosition = { 0.0f, 0.0f };

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

    void UpdateInput(float centerX, float centerY);

    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string_view _title;

    std::array<bool, 512> _keys{};
    std::array<bool, 512> _buttons{};
    bool _isCaptured = false;

};
