//+private
package d3d11

import d3d "vendor:directx/d3d11"

DeviceContext :: struct {
	draw_vertices : u32,
	active_pipeline : ^Pipeline,
	device_context : ^d3d.IDeviceContext,
}

Clear :: proc (device_context : ^DeviceContext, render_target_view : ^^d3d.IRenderTargetView, clear_color : ^[4]f32) {
	device_context.device_context->ClearRenderTargetView(render_target_view^, clear_color)
	device_context.device_context->OMSetRenderTargets(1, render_target_view, nil)
}

SetPipeline :: proc (using _device_context : ^DeviceContext, pipeline : ^Pipeline) {
	active_pipeline = pipeline
	device_context->IASetInputLayout(active_pipeline.input_layout)
	device_context->IASetPrimitiveTopology(pipeline.primitive_topology)
	device_context->VSSetShader(pipeline.vertex_shader, nil, 0)
	device_context->PSSetShader(pipeline.pixel_shader, nil, 0)

	for resource_desc, resource in pipeline.resources {
		switch resource_desc.resource_type {
			case .Sampler:
				val := pipeline.resources[resource_desc]
				device_context->PSSetSamplers(resource_desc.slot_index, 1, (^^d3d.ISamplerState)(&val))
			case .Texture:
				val := pipeline.resources[resource_desc]
				device_context->PSSetShaderResources(resource_desc.slot_index, 1, (^^d3d.IShaderResourceView)(&val))
			case .Buffer:
				// unimplemented
		}
	}

	device_context->RSSetViewports(1, &pipeline.viewport)
}

SetVertexBuffer :: proc (using _device_context : ^DeviceContext, triangle_vertices : ^^d3d.IBuffer, vertex_offset : ^u32) {
	desc : d3d.BUFFER_DESC
	triangle_vertices^->GetDesc(&desc)
	
	device_context->IASetVertexBuffers(0, 1, triangle_vertices, &active_pipeline.vertex_size, vertex_offset)
	draw_vertices = desc.ByteWidth / active_pipeline.vertex_size
}

Draw :: proc (using _device_context : ^DeviceContext) {
	device_context->Draw(draw_vertices, 0)
}

Flush :: proc (using _device_context : ^DeviceContext) {
	device_context->Flush()
}