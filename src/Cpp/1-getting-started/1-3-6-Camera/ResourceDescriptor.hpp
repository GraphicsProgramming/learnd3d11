#pragma once

#include <unordered_map>
#include <cstdint>

enum class ResourceType : uint32_t
{
    Texture,
    Sampler,
    Buffer
};

enum class ResourceStage : uint32_t
{
    VertexStage,
    PixelStage
};

struct ResourceDescriptor {
    ResourceType type;
    ResourceStage stage;
    uint32_t slotIndex;
};

namespace std
{
    template <>
    struct hash<ResourceDescriptor>
    {
        size_t operator ()(const ResourceDescriptor& resource) const noexcept
        {
            const hash<uint64_t> hash;
            return hash(static_cast<uint64_t>(resource.slotIndex) << 32) ^ hash(static_cast<uint32_t>(resource.type));
        }
    };

    template <>
    struct equal_to<ResourceDescriptor>
    {
        bool operator ()(
            const ResourceDescriptor& lhs,
            const ResourceDescriptor& rhs) const noexcept
        {
            return
                lhs.slotIndex == rhs.slotIndex &&
                lhs.type == rhs.type;
        }
    };
}
