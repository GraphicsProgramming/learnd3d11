#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <string_view>

class Input;

class Application
{
public:
    Application(
        const std::int32_t width,
        const std::int32_t height,
        const std::string_view title);
    virtual ~Application();
    void Run();

protected:
    virtual void Cleanup();
    void Close();
    virtual bool Initialize();
    [[nodiscard]] bool IsButtonPressed(const std::int32_t button) const;
    [[nodiscard]] bool IsKeyDown(const std::int32_t key) const;
    [[nodiscard]] bool IsKeyPressed(const std::int32_t key) const;
    [[nodiscard]] bool IsKeyUp(const std::int32_t key) const;
    virtual void Render() = 0;
    virtual void Update() = 0;

    std::unique_ptr<Input> _input;

private:
    GLFWwindow* _window;
    std::int32_t _width;
    std::int32_t _height;
    std::string_view _title;
};
