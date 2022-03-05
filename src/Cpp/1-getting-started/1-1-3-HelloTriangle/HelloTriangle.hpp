#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>

#include "Application.hpp"

#include <string_view>

class HelloTriangleApplication final : public Application
{
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
    HelloTriangleApplication(const std::string_view title);
    ~HelloTriangleApplication() override;

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

    ComPtr<ID3D11Device> _device = nullptr;
#if !defined(NDEBUG)
    ComPtr<ID3D11Debug> _debug = nullptr;
    ComPtr<ID3D11InfoQueue> _debugInfoQueue = nullptr;
#endif
    ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
    ComPtr<IDXGIFactory1> _dxgiFactory = nullptr;
    ComPtr<IDXGISwapChain> _swapChain = nullptr;
    ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    ComPtr<ID3D11Buffer> _triangleVertices = nullptr;
    ComPtr<ID3D11InputLayout> _vertexLayout = nullptr;
    ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
};
