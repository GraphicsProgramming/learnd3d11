package d3d11

import "core:fmt"
import win "core:sys/windows"
import "vendor:glfw"
import "vendor:directx/dxgi"
import d3d "vendor:directx/d3d11"
import "core:math/linalg/hlsl"

import "../../../../framework"

Renderer :: struct {
	factory : ^dxgi.IFactory2,
	device : ^d3d.IDevice,
	swapchain : ^dxgi.ISwapChain1,
	render_target_view : ^d3d.IRenderTargetView,

	triangle_vertices : ^d3d.IBuffer,

	device_context : DeviceContext,
	pipeline : Pipeline,
}

@(private)
VertexType :: enum {
	PositionColor,
}

@(private)
VertexPositionColor :: struct {
	position : hlsl.float3,
	color : hlsl.float3,
}

@(private)
vertex_input_layout_info : []d3d.INPUT_ELEMENT_DESC = {
	{ "POSITION", 0, .R32G32B32_FLOAT, 0, 0, .VERTEX_DATA, 0 },
	{ "COLOR", 0, .R32G32B32_FLOAT, 0, d3d.APPEND_ALIGNED_ELEMENT, .VERTEX_DATA, 0 },
}

@(private)
vertices := [?]VertexPositionColor {
	{ {0.0, 0.5, 0.0}, {0.25, 0.39, 0.19} },
	{ {0.5, -0.5, 0.0}, {0.44, 0.75, 0.35} },
	{ {-0.5, -0.5, 0.0}, {0.38, 0.55, 0.2} },
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

Load :: proc (using renderer : ^Renderer) -> (ok : b32) {
	// Create Pipeline
	pipeline_settings_desc := (PipelineDescriptor) {
		vertex_file_path = "main.vs.hlsl",
		pixel_file_path = "main.ps.hlsl",
		vertex_type = .PositionColor,
	}
	pipeline = CreatePipeline(device, pipeline_settings_desc) or_return

	// Create Vertex Buffer
	buffer_info : d3d.BUFFER_DESC = {
		ByteWidth = size_of(vertices),
		Usage = d3d.USAGE.IMMUTABLE,
		BindFlags = d3d.BIND_FLAG.VERTEX_BUFFER,
	}

	resource_data : d3d.SUBRESOURCE_DATA = {
		pSysMem = &vertices[0],
		SysMemPitch = size_of(vertices),
	}

	result := device->CreateBuffer(&buffer_info, &resource_data, &triangle_vertices)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create triangle vertex data %v\n", u32(result))
		return false
	}

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
}


Render :: proc (app : ^framework.Application, using renderer : ^Renderer) {
	SetViewport(&pipeline, 0.0, 0.0, f32(app.dimensions.x), f32(app.dimensions.y))

	clear_color := [?]f32{ 50.0 / 256.0, 125.0 / 256.0, 250.0 / 256.0, 1.0 }

	vertex_offset : u32 = 0

	Clear(&device_context, &render_target_view, &clear_color)
	SetPipeline(&device_context, &pipeline)
	SetVertexBuffer(&device_context, &triangle_vertices, &vertex_offset)
	Draw(&device_context)

	swapchain->Present(1, 0)
}