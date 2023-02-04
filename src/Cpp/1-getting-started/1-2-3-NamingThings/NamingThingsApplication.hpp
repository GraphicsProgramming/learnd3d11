#pragma once

#include "Definitions.hpp"
#include <Application.hpp>

#include <d3d11_2.h>

#include <memory>

#include "ShaderCollection.hpp"

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

    WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11Buffer> _triangleVertices = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    ShaderCollection _shaderCollection;
};
