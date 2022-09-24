#pragma once

#include "Definitions.hpp"
#include "ResourceDescriptor.hpp"

#include <d3d11_2.h>

#include <cstdint>
#include <unordered_map>

class Pipeline
{
public:
    friend class PipelineFactory;
    friend class DeviceContext;

    void BindTexture(uint32_t slotIndex, ID3D11ShaderResourceView* texture);
    void BindSampler(uint32_t slotIndex, ID3D11SamplerState* sampler);
    void BindVertexStageConstantBuffer(uint32_t slotIndex, ID3D11Buffer* buffer);
    void SetViewport(
        float left,
        float top,
        float width,
        float height);
    void SetDepthStencilState(ID3D11DepthStencilState* depthStencilState);
    void SetRasterizerState(ID3D11RasterizerState* rasterizerState);

private:
    WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
    WRL::ComPtr<ID3D11InputLayout> _inputLayout = nullptr;
    WRL::ComPtr<ID3D11DepthStencilState> _depthStencilState = nullptr;
    WRL::ComPtr<ID3D11RasterizerState> _rasterizerState = nullptr;
    std::unordered_map<ResourceDescriptor, ID3D11DeviceChild*> _resources;
    D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology = {};
    uint32_t _vertexSize = 0;
    D3D11_VIEWPORT _viewport = {};
};
