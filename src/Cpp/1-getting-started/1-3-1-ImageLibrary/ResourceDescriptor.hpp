#pragma once

#include <unordered_map>
#include <cstdint>

enum class ResourceType : uint32_t
{
    Texture,
    Sampler,
    Buffer
};

struct ResourceDescriptor
{
    ResourceType Type;
    uint32_t SlotIndex;
};

namespace std
{
    template <>
    struct hash<ResourceDescriptor>
    {
        size_t operator ()(const ResourceDescriptor& resource) const noexcept
        {
            const hash<uint64_t> hash;
            return hash(static_cast<uint64_t>(resource.SlotIndex) << 32) ^ hash(static_cast<uint32_t>(resource.Type));
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
                lhs.SlotIndex == rhs.SlotIndex &&
                lhs.Type == rhs.Type;
        }
    };
}
