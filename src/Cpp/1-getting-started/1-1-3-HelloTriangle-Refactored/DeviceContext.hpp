#pragma once

#include "Definitions.hpp"

#include <d3d11_2.h>

#include <cstdint>

class Pipeline;

class DeviceContext
{
public:
    DeviceContext(WRL::ComPtr<ID3D11DeviceContext>&& deviceContext);

    void Clear(
        ID3D11RenderTargetView* renderTarget,
        const float clearColor[4]);
    void SetPipeline(const Pipeline* pipeline);
    void SetVertexBuffer(
        ID3D11Buffer* triangleVertices,
        uint32_t vertexOffset);
    void SetViewport(D3D11_VIEWPORT viewport);
    void Draw();
    void Flush();

private:
    uint32_t _drawVertices;
    const Pipeline* _activePipeline;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext;
};
