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
        const float clearColor[4]);
    void SetPipeline(const Pipeline* pipeline);
    void SetVertexBuffer(
        ID3D11Buffer* vertexBuffer,
        uint32_t vertexOffset);
    void SetIndexBuffer(
        ID3D11Buffer* indexBuffer,
        uint32_t indexOffset);
    void SetViewport(D3D11_VIEWPORT viewport);
    void UpdateSubresource(ID3D11Buffer* buffer, const void* data);
    
    void Draw();
    void DrawIndexed();
    void Flush();

private:
    uint32_t _drawVertices;
    uint32_t _drawIndices;
    const Pipeline* _activePipeline;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext;
};
