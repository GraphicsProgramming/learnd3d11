#pragma once

#include "Application.hpp"

#include <memory>

class D3DContext;
struct GraphicsPipeline;
struct StaticMesh;
class HelloTriangleApplication final : public Application
{
public:
    HelloTriangleApplication(const std::string_view title);
    ~HelloTriangleApplication() override;
    bool Initialize() override;

protected:
    void Update() override;
    void Render() override;
private:
    std::unique_ptr<D3DContext> _dxContext;
    std::unique_ptr<GraphicsPipeline> _pipeline;
    std::unique_ptr<StaticMesh> _triangle;
};
