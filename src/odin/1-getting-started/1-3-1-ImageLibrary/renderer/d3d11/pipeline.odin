//+private
package d3d11

import "core:fmt"
import "core:strings"
import win "core:sys/windows"
import d3d "vendor:directx/d3d11"
import d3dc "vendor:directx/d3d_compiler"

PipelineDescriptor :: struct {
	vertex_file_path : string,
	pixel_file_path : string,
	vertex_type : VertexType,
}

Pipeline :: struct {
	vertex_shader : ^d3d.IVertexShader,
	pixel_shader : ^d3d.IPixelShader,
	input_layout : ^d3d.IInputLayout,
	primitive_topology : d3d.PRIMITIVE_TOPOLOGY,
	vertex_size : u32,
	viewport : d3d.VIEWPORT,
}

@(private="file")
input_layout_map := map[VertexType][]d3d.INPUT_ELEMENT_DESC {
	.PositionColor = {
		{ "POSITION", 0, .R32G32B32_FLOAT, 0, 0, .VERTEX_DATA, 0 },
		{ "COLOR", 0, .R32G32B32_FLOAT, 0, d3d.APPEND_ALIGNED_ELEMENT, .VERTEX_DATA, 0 },
	},
}

CreatePipeline :: proc (device : ^d3d.IDevice, settings : PipelineDescriptor) -> (pipeline : Pipeline, ok : b32) {
	vertex_shader_blob : ^d3d.IBlob
	pipeline.vertex_shader, vertex_shader_blob = CreateVertexShader(device, settings.vertex_file_path)
	pipeline.input_layout, ok = CreateInputLayout(device, settings.vertex_type, vertex_shader_blob)
	if !ok {
		fmt.printf("Renderer: Pipeline Creation failed, bad input layout\n")
		return pipeline, false	
	}
	
	pipeline.pixel_shader = CreatePixelShader(device, settings.pixel_file_path)
	pipeline.primitive_topology = .TRIANGLELIST
	pipeline.vertex_size = GetLayoutByteSize(settings.vertex_type)
	return pipeline, true
}

SetViewport :: proc (pipeline : ^Pipeline, x : f32, y : f32, width : f32, height : f32) {
	using pipeline.viewport
	TopLeftX = x
	TopLeftY = y
	Width = width
	Height = height
	MinDepth = 0.0
	MaxDepth = 1.0
}

@(private="file")
CreateInputLayout :: proc (device : ^d3d.IDevice, vertex_type : VertexType, vertex_shader_blob : ^d3d.IBlob) -> (input_layout : ^d3d.IInputLayout, ok : b32) {
	input_layout_desc := input_layout_map[vertex_type]
	result := device->CreateInputLayout(&input_layout_desc[0], u32(len(input_layout_desc)), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create the input layout %v\n", u32(result))
		return nil, false
	}
	return input_layout, true
}

@(private="file")
GetLayoutByteSize :: proc (vertex_type : VertexType) -> (byte_size : u32) {
	switch vertex_type {
		case .PositionColor:
			byte_size = size_of(VertexPositionColor)
	}
	return
}

@(private="file")
CompileShader :: proc (file_name : string, entry_point : string, profile : string) -> (shader_blob : ^d3d.IBlob) {
	compiler_flags := d3dc.D3DCOMPILE.ENABLE_STRICTNESS
	error_blob : ^d3d.IBlob

	wchar_file_name := win.utf8_to_wstring(file_name)
	cstr_entry_point := strings.clone_to_cstring(entry_point)
	cstr_profile := strings.clone_to_cstring(profile)

	/* If there is ever an issue with #includes in the shader, its probably the third argument here, which in a C/C++ codebase would supply D3D_COMPILE_STANDARD_FILE_INCLUDE */
	result := d3dc.CompileFromFile(wchar_file_name, nil, nil, cstr_entry_point, cstr_profile, u32(compiler_flags), 0, &shader_blob, &error_blob)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to read shader from file %v\n", u32(result))
		if error_blob != nil {
			msg := error_blob->GetBufferPointer()
			fmt.printf("\tWith message: %v\n", cstring(msg))
		}
		return nil
	}

	return shader_blob
}

@(private="file")
CreateVertexShader :: proc (device : ^d3d.IDevice, file_name : string) -> (vertex_shader : ^d3d.IVertexShader, vertex_shader_blob : ^d3d.IBlob) {
	vertex_shader_blob = CompileShader(file_name, "Main", "vs_5_0")
	if vertex_shader_blob == nil {
		return nil, nil
	}

	result := device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nil, &vertex_shader)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to compile vertex shader %v\n", u32(result))
		return nil, nil
	}

	return vertex_shader, vertex_shader_blob
}

@(private="file")
CreatePixelShader :: proc (device : ^d3d.IDevice, file_name : string) -> (pixel_shader : ^d3d.IPixelShader) {
	blob := CompileShader(file_name, "Main", "ps_5_0")
	if blob == nil {
		return nil
	}

	result := device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nil, &pixel_shader)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to compile pixel shader %v\n", u32(result))
		return nil
	}

	return pixel_shader
}