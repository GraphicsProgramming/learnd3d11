#include "Pipeline.hpp"

void Pipeline::BindTexture(uint32_t slotIndex, ID3D11ShaderResourceView* texture)
{
    ResourceDescriptor descriptor = {};
    descriptor.slotIndex = slotIndex;
    descriptor.type = ResourceType::Texture;
    _resources[descriptor] = static_cast<ID3D11DeviceChild*>(texture);
}

void Pipeline::BindSampler(uint32_t slotIndex, ID3D11SamplerState* sampler)
{
    ResourceDescriptor descriptor = {};
    descriptor.slotIndex = slotIndex;
    descriptor.type = ResourceType::Sampler;
    _resources[descriptor] = static_cast<ID3D11DeviceChild*>(sampler);
}

void Pipeline::BindVertexStageConstantBuffer(uint32_t slotIndex, ID3D11Buffer* buffer)
{
    ResourceDescriptor descriptor = {};
    descriptor.slotIndex = slotIndex;
    descriptor.stage = ResourceStage::VertexStage;
    descriptor.type = ResourceType::Buffer;
    _resources[descriptor] = static_cast<ID3D11DeviceChild*>(buffer);
}

void Pipeline::SetViewport(
    const float left,
    const float top,
    const float width,
    const float height)
{
    _viewport.TopLeftX = left;
    _viewport.TopLeftY = top;
    _viewport.Width = width;
    _viewport.Height = height;
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;
}

void Pipeline::SetDepthStencilState(ID3D11DepthStencilState* depthStencilState)
{
    _depthStencilState = depthStencilState;
}

void Pipeline::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
    _rasterizerState = rasterizerState;
}
