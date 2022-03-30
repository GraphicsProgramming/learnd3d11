#pragma once

#include <cstdint>
#include <string>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

class Application
{
public:
    Application(const std::string& title);
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
    virtual void Render() = 0;
    virtual void Update();

    [[nodiscard]] GLFWwindow* GetWindow() const;
    [[nodiscard]] int32_t GetWindowWidth() const;
    [[nodiscard]] int32_t GetWindowHeight() const;

private:
    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string _title;
};
