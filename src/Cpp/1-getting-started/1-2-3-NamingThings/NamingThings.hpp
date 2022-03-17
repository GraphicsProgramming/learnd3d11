#pragma once

#include <dxgi1_3.h>
#include <d3d11.h>
#include <wrl.h>

#include "Application.hpp"
#include "Definitions.hpp"

#include <string_view>

class NamingThingsApplication final : public Application
{
public:
    NamingThingsApplication(const std::string_view title);
    ~NamingThingsApplication() override;

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
    WRL::ComPtr<ID3DBlob>& shaderBlob) const;
    [[nodiscard]] WRL::ComPtr<ID3D11VertexShader> CreateVertexShader(
        const std::wstring_view fileName,
        WRL::ComPtr<ID3DBlob>& vertexShaderBlob) const;
    [[nodiscard]] WRL::ComPtr<ID3D11PixelShader> CreatePixelShader(std::wstring_view fileName) const;

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
};
