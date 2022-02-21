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
        const std::int32_t key,
        const std::int32_t scanCode,
        const std::int32_t action,
        const std::int32_t modifier);

    Keyboard() = default;
    ~Keyboard() = default;

    [[nodiscard]] bool IsKeyDown(const std::int32_t key) const;
    [[nodiscard]] bool IsKeyPressed(const std::int32_t key) const;
    [[nodiscard]] bool IsKeyUp(const std::int32_t key) const;

    void Update();
    void HandleKey(std::int32_t key, std::int32_t action);

private:
    std::set<std::int32_t> _keysDown;
    std::set<std::int32_t> _keysUp;
    std::set<std::int32_t> _keysPressed;
    std::set<std::int32_t> _keysReleased;
};
