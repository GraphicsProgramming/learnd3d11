#pragma once

#include "Application.hpp"

// TODO: Maybe don't include this and add forward declarations with unique pointers.
#include "Graphics/D3DContext.hpp"

class HelloTriangleApplication final : public Application
{
public:
    HelloTriangleApplication(const std::string_view title);
    bool Initialize() override;

protected:
    void Update() override;
    void Render() override;
private:
    D3DContext _dxContext;
    GraphicsPipeline _pipeline;
};
