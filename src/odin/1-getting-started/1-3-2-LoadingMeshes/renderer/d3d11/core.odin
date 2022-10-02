package d3d11

import "core:fmt"
import win "core:sys/windows"
import "core:image"
import "core:image/png"
import "core:bytes"
import "vendor:glfw"
import "vendor:directx/dxgi"
import d3d "vendor:directx/d3d11"
import "core:math"
import "core:math/linalg"
import "core:math/linalg/hlsl"

import "../../../../framework"

Renderer :: struct {
	factory : ^dxgi.IFactory2,
	device : ^d3d.IDevice,
	swapchain : ^dxgi.ISwapChain1,
	render_target_view : ^d3d.IRenderTargetView,

	model_vertices : ^d3d.IBuffer,
	model_vertex_count : u32,
	model_indices : ^d3d.IBuffer,
	model_index_count : u32,

	device_context : DeviceContext,
	pipeline : Pipeline,

	linear_sampler_state : ^d3d.ISamplerState,
	texture_srv : ^d3d.IShaderResourceView,
	constant_buffers : [len(ConstantBufferType)]^d3d.IBuffer,

	projection_matrix : matrix[4,4]f32,
	view_matrix : matrix[4,4]f32,
	world_matrix : matrix[4,4]f32,


	// for testing
	angle : f32,
}

@(private)
ConstantBufferType :: enum {
	PerApplication,
	PerFrame,
	PerObject,
}

@(private)
VertexType :: enum {
	PositionColor,
	PositionColorUv,
}

@(private)
VertexPositionColor :: struct {
	position : hlsl.float3,
	color : hlsl.float3,
}

@(private)
VertexPositionColorUv :: struct {
	position : hlsl.float3,
	color : hlsl.float3,
	uv : hlsl.float2,
}

@(private)
vertices := [?]VertexPositionColorUv {
	{ {-0.5, -0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0} },
	{ {0.5, -0.5, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0} },
	{ {-0.5, 0.5, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0} },
	{ {0.5, 0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0} },
}

@(private)
indices := [?]u32 {
	0, 1, 2,
	3, 2, 1,
}

Initialize :: proc (app : ^framework.Application, using renderer : ^Renderer) -> (ok : b32) {
	// Init device, device_context and swapchain
	result := dxgi.CreateDXGIFactory2(0, dxgi.IFactory2_UUID, (^rawptr)(&factory))
	if  !win.SUCCEEDED(result) {
		fmt.printf("DXGI: Failed to create dxgi factory 2 %v\n", u32(result))
		return false
	}

	feature_level := [?]d3d.FEATURE_LEVEL { ._11_0 }
	device_flags := d3d.CREATE_DEVICE_FLAGS { .BGRA_SUPPORT }
	result = d3d.CreateDevice(nil, .HARDWARE, nil, device_flags, &feature_level[0], len(feature_level), d3d.SDK_VERSION, &device, nil, &device_context.device_context)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create Device and Device Context %v\n", u32(result))
		return false
	}

	swapchain_desc := dxgi.SWAP_CHAIN_DESC1 {
		Width = app.dimensions.x,
		Height = app.dimensions.y,
		Format = .B8G8R8A8_UNORM,
		SampleDesc = { Count = 1, Quality = 0, },
		BufferUsage = .RENDER_TARGET_OUTPUT,
		BufferCount = 2,
		Scaling = .STRETCH,
		SwapEffect = .FLIP_DISCARD,
		AlphaMode = .UNSPECIFIED,
		Flags = 0,
	}

	swapchain_fullscreen_desc := dxgi.SWAP_CHAIN_FULLSCREEN_DESC{
		Windowed = true,
	}

	hwnd := glfw.GetWin32Window(app.window)
	result = factory->CreateSwapChainForHwnd(device, hwnd, &swapchain_desc, &swapchain_fullscreen_desc, nil, &swapchain)
	if !win.SUCCEEDED(result) {
		fmt.printf("DXGI: Failed to create Swapchain %v\n", u32(result))
		return false
	}

	CreateSwapchainResources(renderer) or_return

	return true
}

Load :: proc (app : ^framework.Application, using renderer : ^Renderer) -> (ok : b32) {
	// Create Pipeline
	pipeline_settings_desc := (PipelineDescriptor) {
		vertex_file_path = "Assets/Shaders/main.vs.hlsl",
		pixel_file_path = "Assets/Shaders/main.ps.hlsl",
		vertex_type = .PositionColorUv,
	}
	pipeline = CreatePipeline(device, pipeline_settings_desc) or_return

	SetViewport(&pipeline, 0, 0, f32(app.dimensions.x), f32(app.dimensions.y))

	// Load Image
	texture_srv = CreateShaderResourceViewFromFile(device, "Assets/Textures/T_Froge.png")
	if texture_srv == nil {
		fmt.printf("D3D11: Failed to create shader resource view, see above reason\n")
	}
	else {
		BindTexture(&pipeline, 0, texture_srv)
	}

	// Create Sampler
	linear_sampler_desc : d3d.SAMPLER_DESC = {
		Filter = d3d.FILTER.ANISOTROPIC,
		AddressU = d3d.TEXTURE_ADDRESS_MODE.WRAP,
		AddressV = d3d.TEXTURE_ADDRESS_MODE.WRAP,
		AddressW = d3d.TEXTURE_ADDRESS_MODE.WRAP,
		ComparisonFunc = d3d.COMPARISON_FUNC.NEVER,
	}

	result := device->CreateSamplerState(&linear_sampler_desc, &linear_sampler_state)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create linear sampler state, %v\n", u32(result))
		return false
	}

	BindSampler(&pipeline, 0, linear_sampler_state)

	// Load Model
	model_vertices, model_vertex_count, model_indices, model_index_count = LoadModel(device, "Assets/Models/test.txt")

	constant_buffer_desc : d3d.BUFFER_DESC = {
		ByteWidth = size_of(matrix[4,4]f32),
		BindFlags = .CONSTANT_BUFFER,
	}

	result = device->CreateBuffer(&constant_buffer_desc, nil, &constant_buffers[ConstantBufferType.PerApplication])
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create constant buffer PerApplication, %v\n", u32(result))
		return false
	}

	result = device->CreateBuffer(&constant_buffer_desc, nil, &constant_buffers[ConstantBufferType.PerFrame])
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create constant buffer PerFrame, %v\n", u32(result))
		return false
	}

	result = device->CreateBuffer(&constant_buffer_desc, nil, &constant_buffers[ConstantBufferType.PerObject])
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create constant buffer PerObject, %v\n", u32(result))
		return false
	}

	BindVertexStageConstantBuffer(&pipeline, 0, constant_buffers[ConstantBufferType.PerApplication])
	BindVertexStageConstantBuffer(&pipeline, 1, constant_buffers[ConstantBufferType.PerFrame])
	BindVertexStageConstantBuffer(&pipeline, 2, constant_buffers[ConstantBufferType.PerObject])

	// this isn't in the right NDC, will need to do some conversion
	projection_matrix = linalg.matrix4_perspective(math.to_radians(f32(45.0)), f32(app.dimensions.x) / f32(app.dimensions.y), 0.1, 512)
	UpdateSubResource(&device_context, constant_buffers[ConstantBufferType.PerApplication], &projection_matrix)

	return true
}

@(private)
CreateSwapchainResources :: proc (using renderer : ^Renderer) -> (ok : b32) {
	backbuffer : ^d3d.ITexture2D = nil
	result := swapchain->GetBuffer(0, d3d.ITexture2D_UUID, (^rawptr)(&backbuffer))
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to get Backbuffer from swapchain %v\n", u32(result))
		return false
	}
	defer backbuffer->Release()

	result = device->CreateRenderTargetView(backbuffer, nil, &render_target_view)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create RTV from Backbuffer %v\n", u32(result))
		return false
	}

	return true
}

@(private)
DestroySwapchainResources :: proc (using renderer : ^Renderer) {
	device_context.device_context->OMSetRenderTargets(0, nil, nil)
	if render_target_view != nil {
		render_target_view->Release()
		render_target_view = nil
	}
}

OnResize :: proc (app : ^framework.Application, using renderer : ^Renderer, new_width : u32, new_height : u32) {
	app.dimensions = {new_width, new_height}
	Flush(&device_context)

	DestroySwapchainResources(renderer)

	result := swapchain->ResizeBuffers(0, app.dimensions.x, app.dimensions.y, .UNKNOWN, 0)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to recreate Swapchain buffers %x\n", u32(result))
		return
	}

	CreateSwapchainResources(renderer)

	// this isn't in the right NDC, will need to do some conversion
	projection_matrix = linalg.matrix4_perspective(math.to_radians(f32(45.0)), f32(new_width) / f32(new_height), 0.1, 512)
	UpdateSubResource(&device_context, constant_buffers[ConstantBufferType.PerApplication], &projection_matrix)
}

Update :: proc (app : ^framework.Application, using renderer : ^Renderer) {
	eye_pos : linalg.Vector3f32 = { 0.0, 0.0, 10.0 }
	focus_point : linalg.Vector3f32 = { 0.0, 0.0, 0.0 }
	up_direction : linalg.Vector3f32 = { 0.0, 1.0, 0.0 }

	view_matrix = linalg.matrix4_look_at_f32(eye_pos, focus_point, up_direction)
	UpdateSubResource(&device_context, constant_buffers[ConstantBufferType.PerFrame], &view_matrix)

	angle += 90.0 * (1000.0 / 60000.0)
	rotation_axis := up_direction

	world_matrix = linalg.matrix4_rotate_f32(math.to_radians_f32(angle), rotation_axis)
	UpdateSubResource(&device_context, constant_buffers[ConstantBufferType.PerObject], &world_matrix)
}

Render :: proc (app : ^framework.Application, using renderer : ^Renderer) {
	SetViewport(&pipeline, 0.0, 0.0, f32(app.dimensions.x), f32(app.dimensions.y))

	clear_color := [?]f32{ 50.0 / 256.0, 125.0 / 256.0, 250.0 / 256.0, 1.0 }

	vertex_offset : u32 = 0

	Clear(&device_context, &render_target_view, &clear_color)
	SetPipeline(&device_context, &pipeline)
	SetVertexBuffer(&device_context, &model_vertices, &vertex_offset)
	SetIndexBuffer(&device_context, model_indices, 0)
	DrawIndexed(&device_context)

	swapchain->Present(1, 0)
}

CreateShaderResourceViewFromFile :: proc (device : ^d3d.IDevice, file_path : string) -> (shader_resource_view : ^d3d.IShaderResourceView) {
	// This implementation is temporary.
	image_options : image.Options = { .return_metadata }
	img, err := image.load_from_file(file_path, image_options)
	if err != nil {
		fmt.printf("IMAGE LOAD: Failed to load image: %v, Error: %v\n", file_path, err)
		return nil
	}

	texture_desc : d3d.TEXTURE2D_DESC = {
		Width = u32(img.width),
		Height = u32(img.height),
		MipLevels = 1,
		ArraySize = 1,
		Format = dxgi.FORMAT.R8G8B8A8_UNORM,
		SampleDesc = { Count = 1, Quality = 0},
		Usage = d3d.USAGE.IMMUTABLE,
		BindFlags = d3d.BIND_FLAG.SHADER_RESOURCE,
	}
	
	byte_arr := bytes.buffer_to_bytes(&img.pixels)
	texture_data : d3d.SUBRESOURCE_DATA = {
		pSysMem = &byte_arr[0],
		SysMemPitch = u32(img.width * img.channels),
	}

	texture : ^d3d.ITexture2D
	result := device->CreateTexture2D(&texture_desc, &texture_data, &texture)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create texture from %v, Error: %v\n", file_path, u32(result))
		return nil
	}

	result = device->CreateShaderResourceView(texture, nil, &shader_resource_view)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create shader resource view for %v, Error: %v\n", file_path, u32(result))
		return nil
	}

	return
}

LoadModel :: proc(device : ^d3d.IDevice, file_path : string) -> (vertex_buffer : ^d3d.IBuffer, vertex_count : u32, index_buffer : ^d3d.IBuffer, index_count : u32) {
	fmt.printf("LOAD MODEL: Model Loading is not implemented yet. File Path: %v, was not loaded. Falling back to default triangle.\n", file_path)
	
	// Loading a model would go here.

	// Create vertex and index buffers.
	vertex_buffer_desc : d3d.BUFFER_DESC = {
		ByteWidth = size_of(VertexPositionColorUv) * len(vertices),
		Usage = .IMMUTABLE,
		BindFlags = .VERTEX_BUFFER,
	}

	fmt.printf("DEBUG: ByteWidth = size_of(VertexPositionColorUv) * len(vertices), value : %v\n", size_of(VertexPositionColorUv) * len(vertices))

	vertex_buffer_data : d3d.SUBRESOURCE_DATA = {
		pSysMem = &vertices[0],
	}

	result := device->CreateBuffer(&vertex_buffer_desc, &vertex_buffer_data, &vertex_buffer)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create vertex buffer, Error: %v\n", u32(result))
		return nil, 0, nil, 0
	}
	vertex_count = u32(len(vertices))

	fmt.printf("DEBUG: vertex_count = u32(len(vertices)), %v\n", vertex_count)

	index_buffer_desc : d3d.BUFFER_DESC = {
		ByteWidth = size_of(u32) * len(indices),
		Usage = .IMMUTABLE,
		BindFlags = .INDEX_BUFFER,
	}

	fmt.printf("DEBUG: ByteWidth = size_of(u32) * len(indices), value : %v\n", size_of(u32) * len(indices))

	index_buffer_data : d3d.SUBRESOURCE_DATA = {
		pSysMem = &indices[0],
	}

	result = device->CreateBuffer(&index_buffer_desc, &index_buffer_data, &index_buffer)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create index buffer, Error: %v\n", u32(result))
		return nil, 0, nil, 0
	}
	index_count = u32(len(indices))

	fmt.printf("DEBUG: index_count = u32(len(indices)), value: %v\n", index_count)

	return
}