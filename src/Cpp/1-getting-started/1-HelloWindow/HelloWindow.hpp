#pragma once

#include "Application.hpp"

class HelloTriangleApplication final : public Application
{
public:
    HelloTriangleApplication(const std::string_view title);

protected:
    void Update() override;
    void Render() override;
};
