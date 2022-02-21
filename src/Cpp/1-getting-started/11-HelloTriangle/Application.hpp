#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

// FIXME: Is including Windows.h just for HWND good?
#include <Windows.h>

class D3DContext;
// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;
class Input;

class Application
{
public:
    Application(const std::string_view title);
    virtual ~Application();
    // FIXME: Should we have this function here?
    [[nodiscard]] HWND GetWindowHandle() const;
    [[nodiscard]] int32_t GetWindowWidth() const;
    [[nodiscard]] int32_t GetWindowHeight() const;
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
    int32_t _width;
    int32_t _height;
    std::string_view _title;
};
