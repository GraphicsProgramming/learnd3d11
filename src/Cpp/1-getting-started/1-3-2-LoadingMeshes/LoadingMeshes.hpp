#pragma once

#include <dxgi1_3.h>
#include <d3dcommon.h>
#include <d3d11.h>

#include <DirectXMath.h>

#include "Application.hpp"
#include "Definitions.hpp"
#include "ShaderFactory.hpp"
#include "TextureFactory.hpp"

#include <string>
#include <memory>

class LoadingMeshesApplication final : public Application
{
public:
    LoadingMeshesApplication(const std::string_view title);
    ~LoadingMeshesApplication() override;

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

    bool LoadModel(const std::string& filePath);

    enum ConstantBufferType
    {
        PerApplication,
        PerFrame,
        PerObject,
        NumConstantBufferTypes
    };

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11Buffer> _modelVertices = nullptr;
    WRL::ComPtr<ID3D11Buffer> _modelIndices = nullptr;
    WRL::ComPtr<ID3D11InputLayout> _vertexLayout = nullptr;
    WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;
    WRL::ComPtr<ID3D11ShaderResourceView> _textureSrv = nullptr;

    std::unique_ptr<ShaderFactory> _shaderFactory = nullptr;
    std::unique_ptr<TextureFactory> _textureFactory = nullptr;

    WRL::ComPtr<ID3D11Buffer> _constantBuffers[NumConstantBufferTypes];
    DirectX::XMMATRIX _projectionMatrix;
    DirectX::XMMATRIX _viewMatrix;
    DirectX::XMMATRIX _worldMatrix;

    uint32_t _modelVertexCount = 0;
    uint32_t _modelIndexCount = 0;
};
