#pragma once

#include "Definitions.hpp"
#include <Application.hpp>

#include <DirectXMath.h>
#include <d3d11_2.h>

#include <memory>
#include <string>

class Pipeline;
class PipelineFactory;
class DeviceContext;
class TextureFactory;

class LoadingMeshesApplication final : public Application
{
public:
    LoadingMeshesApplication(const std::string& title);
    ~LoadingMeshesApplication() override;

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
    DirectX::XMMATRIX _projectionMatrix = DirectX::XMMATRIX();
    DirectX::XMMATRIX _viewMatrix = DirectX::XMMATRIX();
    DirectX::XMMATRIX _worldMatrix = DirectX::XMMATRIX();

    uint32_t _modelVertexCount = 0;
    uint32_t _modelIndexCount = 0;
};
