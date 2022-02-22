#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include "Application.hpp"

#include <string_view>

class HelloD3D11Application final : public Application
{
public:
    HelloD3D11Application(const std::string_view title);
    ~HelloD3D11Application() override;
    bool Initialize() override;
protected:
    void Update() override;
    void Render() override;
private:
    ID3D11Device*           _device          = nullptr;
#if !defined(NDEBUG)
    ID3D11Debug*            _debug           = nullptr;
    ID3D11InfoQueue*        _debugInfoQueue  = nullptr;
#endif
    ID3D11DeviceContext*    _deviceContext   = nullptr;
    IDXGIFactory1*          _dxgiFactory     = nullptr;
    IDXGISwapChain*         _swapChain       = nullptr;
    ID3D11RenderTargetView* _renderTarget    = nullptr;
    ID3D11Buffer*           _triangleVerts   = nullptr;
    ID3D11Buffer*           _triangleIndices = nullptr;
    ID3D11InputLayout*      _vertexLayout    = nullptr;
    ID3D11VertexShader*     _vertexShader    = nullptr;
    ID3D11PixelShader*      _pixelShader     = nullptr;
};
