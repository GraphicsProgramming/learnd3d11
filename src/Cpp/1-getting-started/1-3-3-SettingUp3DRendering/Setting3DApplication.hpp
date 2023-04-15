#pragma once

#include "Definitions.hpp"
#include <Application.hpp>

#include <d3d11_2.h>

#include <memory>
#include <DirectXMath.h>
#include "ShaderCollection.hpp"

struct PerFrameConstantBuffer
{
    DirectX::XMFLOAT4X4 viewProjectionMatrix;
};

struct PerObjectConstantBuffer
{
    DirectX::XMFLOAT4X4 modelMatrix;
};


class Setting3DApplication final : public Application
{
public:
    Setting3DApplication(const std::string& title);
    ~Setting3DApplication() override;

protected:
    bool Initialize() override;
    bool Load() override;
    void OnResize(
        int32_t width,
        int32_t height) override;
    void Update() override;
    void Render() override;

private:
    void CreateConstantBuffers();
    bool CreateSwapchainResources();
    void DestroySwapchainResources();

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _rasterState = nullptr;
    WRL::ComPtr<ID3D11Buffer> _triangleVertices = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;
    WRL::ComPtr<ID3D11ShaderResourceView> _textureSrv = nullptr;
    WRL::ComPtr<ID3D11ShaderResourceView> _fallbackTextureSrv = nullptr;

    WRL::ComPtr<ID3D11Buffer> _perFrameConstantBuffer = nullptr;
    WRL::ComPtr<ID3D11Buffer> _perObjectConstantBuffer = nullptr;

    ShaderCollection _shaderCollection;

    PerFrameConstantBuffer _perFrameConstantBufferData{};
    PerObjectConstantBuffer _perObjectConstantBufferData{};
};
