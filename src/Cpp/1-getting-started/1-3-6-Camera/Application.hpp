#pragma once

#include <string_view>
#include <cstdint>
#include <memory>

struct GLFWwindow;
class Input;

class Application
{
public:
    Application(const std::string_view title);
    virtual ~Application();

    void Run();
protected:
    static void HandleResize(
        GLFWwindow* window,
        const int32_t width,
        const int32_t height);
    virtual void OnResize(
        const int32_t width,
        const int32_t height);

    virtual bool Initialize();
    virtual bool Load() = 0;
    virtual void Cleanup();
    void Close();
    virtual void Render() = 0;
    virtual void Update() = 0;

    [[nodiscard]] GLFWwindow* GetWindow() const;
    [[nodiscard]] int32_t GetWindowWidth() const;
    [[nodiscard]] int32_t GetWindowHeight() const;

    [[nodiscard]] bool IsButtonPressed(const std::int32_t button) const;
    [[nodiscard]] bool IsKeyDown(const std::int32_t key) const;
    [[nodiscard]] bool IsKeyPressed(const std::int32_t key) const;
    [[nodiscard]] bool IsKeyUp(const std::int32_t key) const;

    std::unique_ptr<Input> _input = nullptr;

private:
    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string_view _title;
};
