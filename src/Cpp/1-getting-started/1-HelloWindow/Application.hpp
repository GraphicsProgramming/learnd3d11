#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;
class Input;

class Application
{
public:
    Application(const std::string_view title);
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

    std::unique_ptr<Input> _input = nullptr;

private:
    GLFWwindow* _window = nullptr;
    std::int32_t _width = 0;
    std::int32_t _height = 0;
    std::string_view _title;
};
