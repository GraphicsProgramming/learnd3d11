#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>

#include "Application.hpp"

#include <string_view>

class HelloD3D11Application final : public Application
{
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
    HelloD3D11Application(const std::string_view title);
    ~HelloD3D11Application() override;
    bool Initialize() override;
protected:
    void OnResize(
        int32_t width,
        int32_t height) override;
    void Update() override;
    void Render() override;
private:
    ComPtr<ID3D11Device>           _device          = nullptr;
#if !defined(NDEBUG)
    ComPtr<ID3D11Debug>            _debug           = nullptr;
    ComPtr<ID3D11InfoQueue>        _debugInfoQueue  = nullptr;
#endif
    ComPtr<ID3D11DeviceContext>    _deviceContext   = nullptr;
    ComPtr<IDXGIFactory1>          _dxgiFactory     = nullptr;
    ComPtr<IDXGISwapChain>         _swapChain       = nullptr;
    ComPtr<ID3D11RenderTargetView> _renderTarget    = nullptr;
};
