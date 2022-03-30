#pragma once

#include <Application.hpp>
#include "Definitions.hpp"

#include <d3d11_2.h>

#include <memory>

class DeviceContext;
class Pipeline;
class PipelineFactory;

class NamingThingsApplication final : public Application
{
public:
    NamingThingsApplication(const std::string& title);
    ~NamingThingsApplication() override;

protected:
    bool Initialize() override;
    bool Load() override;
    void OnResize(
        int32_t width,
        int32_t height) override;
    void Update() override;
    void Render() override;

private:
    bool CreateSwapchainResources();
    void DestroySwapchainResources();

    std::unique_ptr<DeviceContext> _deviceContext = nullptr;
    std::unique_ptr<Pipeline> _pipeline = nullptr;
    std::unique_ptr<PipelineFactory> _pipelineFactory = nullptr;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11Buffer> _triangleVertices = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;
};
