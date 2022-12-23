#pragma once

#include "Definitions.hpp"
#include <Application.hpp>

#include <DirectXMath.h>
#include <d3d11_2.h>

#include <memory>
#include <array>

class Pipeline;
class PipelineFactory;
class DeviceContext;


class RasterizerStateApplication final : public Application
{
public:
    RasterizerStateApplication(const std::string& title);
    ~RasterizerStateApplication() override;

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

    bool CreateRasterizerStates();

    enum ConstantBufferType
    {
        PerApplication,
        PerFrame,
        PerObject,
        NumConstantBufferTypes
    };

    std::unique_ptr<Pipeline> _pipeline = nullptr;
    std::unique_ptr<DeviceContext> _deviceContext = nullptr;
    std::unique_ptr<PipelineFactory> _pipelineFactory = nullptr;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    std::array<WRL::ComPtr<ID3D11Buffer>,6> _triangleVertices;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    WRL::ComPtr<ID3D11RasterizerState> _wireFrameCullBackRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _wireFrameCullFrontRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _wireFrameCullNoneRasterizerState = nullptr;

    WRL::ComPtr<ID3D11RasterizerState> _solidFrameCullBackRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _solidFrameCullFrontRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _solidFrameCullNoneRasterizerState = nullptr;
};
