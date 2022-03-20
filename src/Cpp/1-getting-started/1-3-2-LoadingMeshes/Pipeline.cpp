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
