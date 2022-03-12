#pragma once

#include <string_view>
#include <cstdint>

struct GLFWwindow;
class Application
{
public:
    Application(const std::string_view title);
    virtual ~Application();

    void Run();
protected:
    static void HandleResize(
        GLFWwindow* window,
        int32_t width,
        int32_t height);
    virtual void OnResize(
        int32_t width,
        int32_t height);

    virtual bool Initialize();
    virtual void Cleanup();
    virtual void Render() = 0;
    virtual void Update() = 0;

    [[nodiscard]] GLFWwindow* GetWindow() const;
    [[nodiscard]] int32_t GetWindowWidth() const;
    [[nodiscard]] int32_t GetWindowHeight() const;
    void Close();
    
private:
    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string_view _title;
};