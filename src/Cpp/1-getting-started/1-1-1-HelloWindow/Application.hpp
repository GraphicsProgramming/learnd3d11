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
    virtual bool Initialize();
    virtual bool Load() = 0;
    virtual void Render() = 0;
    virtual void Update() = 0;

private:
    virtual void Cleanup();

    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string _title;
};
