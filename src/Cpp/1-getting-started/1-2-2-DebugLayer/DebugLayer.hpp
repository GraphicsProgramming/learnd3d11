#pragma once

#include <dxgi1_3.h>
#include <d3d11.h>
#include <wrl.h>

#include "Application.hpp"
#include "Definitions.hpp"
#include "ShaderFactory.hpp"

#include <memory>
#include <string_view>

class DebugLayerApplication final : public Application
{
public:
    DebugLayerApplication(const std::string_view title);
    ~DebugLayerApplication() override;

protected:
    bool Initialize() override;
    void OnResize(
        const int32_t width,
        const int32_t height) override;
    void Update() override;
    void Render() override;

private:
    bool CreateSwapchainResources();
    void DestroySwapchainResources();

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11Buffer> _triangleVertices = nullptr;
    WRL::ComPtr<ID3D11InputLayout> _vertexLayout = nullptr;
    WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    std::unique_ptr<ShaderFactory> _shaderFactory = nullptr;
};
