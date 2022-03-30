#pragma once

#include <Application.hpp>
#include "Definitions.hpp"

#include <d3d11_2.h>
#include <DirectXMath.h>

#include <string>
#include <memory>

class Pipeline;
class PipelineFactory;
class DeviceContext;
class TextureFactory;

class LoadingMeshesApplication final : public Application
{
public:
    LoadingMeshesApplication(const std::string_view title);
    ~LoadingMeshesApplication() override;

protected:
    bool Initialize() override;
    bool Load() override;
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

    std::unique_ptr<Pipeline> _pipeline = nullptr;
    std::unique_ptr<DeviceContext> _deviceContext = nullptr;
    std::unique_ptr<PipelineFactory> _pipelineFactory = nullptr;
    std::unique_ptr<TextureFactory> _textureFactory = nullptr;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11Buffer> _modelVertices = nullptr;
    WRL::ComPtr<ID3D11Buffer> _modelIndices = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;
    WRL::ComPtr<ID3D11ShaderResourceView> _textureSrv = nullptr;

    WRL::ComPtr<ID3D11Buffer> _constantBuffers[NumConstantBufferTypes];
    DirectX::XMMATRIX _projectionMatrix;
    DirectX::XMMATRIX _viewMatrix;
    DirectX::XMMATRIX _worldMatrix;

    uint32_t _modelVertexCount = 0;
    uint32_t _modelIndexCount = 0;
};
