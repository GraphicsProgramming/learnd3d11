#include "Pipeline.hpp"

void Pipeline::BindTexture(uint32_t slotIndex, ID3D11ShaderResourceView* texture)
{
    ResourceDescriptor descriptor = {};
    descriptor.SlotIndex = slotIndex;
    descriptor.Type = ResourceType::Texture;
    _resources[descriptor] = static_cast<ID3D11DeviceChild*>(texture);
}

void Pipeline::BindSampler(uint32_t slotIndex, ID3D11SamplerState* sampler)
{
    ResourceDescriptor descriptor = {};
    descriptor.SlotIndex = slotIndex;
    descriptor.Type = ResourceType::Sampler;
    _resources[descriptor] = static_cast<ID3D11DeviceChild*>(sampler);
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
