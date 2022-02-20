#pragma once

#include <cstdint>
#include <set>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

class Keyboard
{
public:
    static void HandleKeys(
        GLFWwindow* window,
        const int32_t key,
        const int32_t scanCode,
        const int32_t action,
        const int32_t modifier);

    Keyboard() = default;
    ~Keyboard() = default;

    [[nodiscard]] bool IsKeyDown(const int32_t key) const;

    [[nodiscard]] bool IsKeyPressed(const int32_t key) const;

    [[nodiscard]] bool IsKeyUp(const int32_t key) const;

    void Update();

    void HandleKey(
        const int32_t key,
        const int32_t action);

private:
    std::set<int32_t> _keysDown;
    std::set<int32_t> _keysUp;
    std::set<int32_t> _keysPressed;
    std::set<int32_t> _keysReleased;
};
