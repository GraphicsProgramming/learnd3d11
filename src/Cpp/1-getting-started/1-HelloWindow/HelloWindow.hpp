#pragma once

#include "Application.hpp"

class HelloWindowApplication : public Application
{
public:
    HelloWindowApplication(
    const std::int32_t width,
    const std::int32_t height,
    const std::string_view title);

protected:
    void Update() override;
    void Render() override;
};
