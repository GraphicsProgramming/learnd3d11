#pragma once

#include <dxgi1_3.h>
#include <d3d11.h>
#include <wrl.h>

#include <Application.hpp>

class HelloTriangleApplication final : public Application
{
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
    HelloTriangleApplication(const std::string& title);
    ~HelloTriangleApplication() override;

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

    bool CompileShader(
        const std::wstring& fileName,
        const std::string& entryPoint,
        const std::string& profile,
        ComPtr<ID3DBlob>& shaderBlob) const;
    [[nodiscard]] ComPtr<ID3D11VertexShader> CreateVertexShader(
        const std::wstring& fileName,
        ComPtr<ID3DBlob>& vertexShaderBlob) const;
    [[nodiscard]] ComPtr<ID3D11PixelShader> CreatePixelShader(const std::wstring& fileName) const;

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
