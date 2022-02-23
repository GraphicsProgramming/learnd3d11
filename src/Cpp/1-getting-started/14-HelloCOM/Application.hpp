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
    virtual bool Initialize();
    virtual void Cleanup();
    virtual void Render() = 0;
    virtual void Update() = 0;

    [[nodiscard]] GLFWwindow* GetWindow() const;
                  void        Close();

    int32_t          _width = 0;
    int32_t          _height = 0;
private:
    GLFWwindow*      _window = nullptr;
    std::string_view _title;
};
