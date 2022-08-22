package d3d11

ResourceType :: enum {
	Texture,
	Sampler,
	Buffer,
}

ResourceDescriptor :: struct {
	resource_type : ResourceType,
	slot_index : u32,
}

// hash for ResourceDescriptor?
// equal_to function for ResourceDescriptor?