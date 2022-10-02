package d3d11

ResourceType :: enum {
	Texture,
	Sampler,
	Buffer,
}

ResourceStage :: enum {
	VertexStage,
	PixelStage,
}

ResourceDescriptor :: struct {
	resource_type : ResourceType,
	resource_stage : ResourceStage,
	slot_index : u32,
}

// hash for ResourceDescriptor?
// equal_to function for ResourceDescriptor?