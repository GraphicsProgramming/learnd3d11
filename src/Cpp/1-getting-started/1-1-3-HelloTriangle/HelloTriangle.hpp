#pragma once

#include <dxgi1_3.h>
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

bool CompileShader(
    const std::wstring_view fileName,
    const std::string_view entryPoint,
    const std::string_view profile,
    ComPtr<ID3D10Blob>& shaderBlob) const;
    [[nodiscard]] ComPtr<ID3D11VertexShader> CreateVertexShader(
        const std::wstring_view fileName,
        ComPtr<ID3D10Blob>& vertexShaderBlob) const;
    [[nodiscard]] ComPtr<ID3D11PixelShader> CreatePixelShader(std::wstring_view fileName) const;

    ComPtr<ID3D11Device> _device = nullptr;
    ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
    ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
    ComPtr<IDXGISwapChain1> _swapChain = nullptr;
    ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
    ComPtr<ID3D11Buffer> _triangleVertices = nullptr;
    ComPtr<ID3D11InputLayout> _vertexLayout = nullptr;
    ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
};
