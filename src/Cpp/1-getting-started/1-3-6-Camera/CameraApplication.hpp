#pragma once

#include "Application.hpp"
#include "Definitions.hpp"

#include <d3d11_2.h>

#include <string_view>
#include <memory>

class Camera;
class Pipeline;
class PipelineFactory;
class DeviceContext;
class TextureFactory;
class ModelFactory;

struct ImGuiContext;

class CameraApplication final : public Application
{
public:
    CameraApplication(const std::string_view title);
    ~CameraApplication() override;

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

    bool CreateDepthStencilStates();
    bool CreateRasterizerStates();

    void InitializeImGui();
    void RenderUi();

    std::unique_ptr<Camera> _camera = nullptr;

    std::unique_ptr<Pipeline> _pipeline = nullptr;
    std::unique_ptr<DeviceContext> _deviceContext = nullptr;
    std::unique_ptr<PipelineFactory> _pipelineFactory = nullptr;
    std::unique_ptr<TextureFactory> _textureFactory = nullptr;
    std::unique_ptr<ModelFactory> _modelFactory = nullptr;

    ImGuiContext* _imGuiContext = nullptr;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView = nullptr;
    WRL::ComPtr<ID3D11Buffer> _modelVertices = nullptr;
    WRL::ComPtr<ID3D11Buffer> _modelIndices = nullptr;
    WRL::ComPtr<ID3D11Debug> _debug = nullptr;

    WRL::ComPtr<ID3D11DepthStencilState> _depthDisabledDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledLessDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledLessEqualDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledAlwaysDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledNeverDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledEqualDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledNotEqualDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledGreaterDepthStencilState = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthEnabledGreaterEqualDepthStencilState = nullptr;

    WRL::ComPtr<ID3D11RasterizerState> _wireFrameCullBackRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _wireFrameCullFrontRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _wireFrameCullNoneRasterizerState = nullptr;

    WRL::ComPtr<ID3D11RasterizerState> _solidFrameCullBackRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _solidFrameCullFrontRasterizerState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _solidFrameCullNoneRasterizerState = nullptr;

    WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;
    WRL::ComPtr<ID3D11ShaderResourceView> _textureSrv = nullptr;
    WRL::ComPtr<ID3D11Buffer> _cameraConstantBuffer = nullptr;
    WRL::ComPtr<ID3D11Buffer> _objectConstantBuffer = nullptr;

    DirectX::XMFLOAT4X4 _worldMatrix = DirectX::XMFLOAT4X4();

    uint32_t _modelVertexCount = 0;
    uint32_t _modelIndexCount = 0;
    bool _toggledRotation = false;
    int32_t _selectedDepthFunction = 1;
    int32_t _selectedRasterizerState = 11;
    bool _isWireframe = false;
};
