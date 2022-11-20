#pragma once

#include "Definitions.hpp"

#include <d3d11_2.h>

#include <cstdint>
#include <map>

class Pipeline;

class DeviceContext
{
public:
    DeviceContext(WRL::ComPtr<ID3D11DeviceContext>&& deviceContext);

    void Clear(
        ID3D11RenderTargetView* renderTarget,
        float clearColor[4]) const;
    void SetPipeline(const Pipeline* pipeline);
    void SetVertexBuffer(
        ID3D11Buffer* triangleVertices,
        uint32_t vertexOffset);
    void Draw() const;
    void Flush() const;

private:
    uint32_t _drawVertices;
    const Pipeline* _activePipeline;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext;
};
