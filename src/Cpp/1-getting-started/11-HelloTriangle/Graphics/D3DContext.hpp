#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <string_view>

#include <wrl/client.h>

class Application;

struct GraphicsPipeline
{
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inpuLayout;
};

class D3DContext
{
public:
    D3DContext() = default;

    [[nodiscard]] bool Initialize(const Application&);
    [[nodiscard]] bool MakeGraphicsPipeline(
        const wchar_t* vertexPath,
        const wchar_t* pixelPath,
        GraphicsPipeline& outPipeline) const;

private:
    // Order of initialization matters for automatic destruction in reverse order
    Microsoft::WRL::ComPtr<ID3D11Device>           _device         = nullptr;
#if !defined(NDEBUG)
    Microsoft::WRL::ComPtr<ID3D11Debug>            _debug          = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InfoQueue>        _debugInfoQueue = nullptr;
#endif
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    _deviceContext  = nullptr;
    Microsoft::WRL::ComPtr<IDXGIFactory1>          _dxgiFactory    = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         _swapChain      = nullptr;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTarget   = nullptr;
};
