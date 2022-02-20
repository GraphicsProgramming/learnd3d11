#pragma once

#include "Application.hpp"

class HelloWindowApplication final : public Application
{
public:
    HelloWindowApplication(const std::string_view title);

protected:
    void Update() override;
    void Render() override;
};
