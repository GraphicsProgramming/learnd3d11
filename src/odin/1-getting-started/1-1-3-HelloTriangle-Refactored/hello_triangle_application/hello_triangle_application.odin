package hello_triangle_application

import "core:fmt"
import win "core:sys/windows"
import "vendor:glfw"
import d3d "vendor:directx/d3d11"
import "vendor:directx/dxgi"
import d3dc "vendor:directx/d3d_compiler"

import "../../../framework"

HelloTriangleApplication :: struct {
	device : ^d3d.IDevice,
	device_context : ^d3d.IDeviceContext,
	swapchain : ^dxgi.ISwapChain1,
	factory : ^dxgi.IFactory2,
	render_target_view : ^d3d.IRenderTargetView,

	using _application : framework.Application,
}

CreateHelloTriangleApplication :: proc (title : string) -> (app : HelloTriangleApplication) {
	app.title = title
	return
}

Run :: proc (app : ^HelloTriangleApplication) {
	defer Cleanup(app)
	if ok := Initialize(app); !ok {
		return
	}
	
	if ok := Load(app); !ok {
		return
	}

	for !glfw.WindowShouldClose(app.window) {
		glfw.PollEvents()
		Update(app)
		Render(app)
	}
}

@(private)
Initialize :: proc (app : ^HelloTriangleApplication) -> (ok : b32) {
	framework.Initialize(app) or_return

	glfw.SetWindowUserPointer(app.window, rawptr(app))
	glfw.SetFramebufferSizeCallback(app.window, glfw.FramebufferSizeProc(ResizeHandler))

	result := dxgi.CreateDXGIFactory2(0, dxgi.IFactory2_UUID, (^rawptr)(&app.factory))
	if  !win.SUCCEEDED(result) {
		fmt.printf("DXGI: Failed to create dxgi factory 2 %v\n", u32(result))
		return false
	}

	feature_level := [?]d3d.FEATURE_LEVEL{ ._11_0 }
	device_flags := d3d.CREATE_DEVICE_FLAGS{ .BGRA_SUPPORT }
	result = d3d.CreateDevice(nil, .HARDWARE, nil, device_flags, &feature_level[0], len(feature_level), d3d.SDK_VERSION, &app.device, nil, &app.device_context)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create Device and Device Context %v\n", u32(result))
		return false
	}

	swapchain_desc := dxgi.SWAP_CHAIN_DESC1{
		Width = u32(app.dimensions.x),
		Height = u32(app.dimensions.y),
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
	result = app.factory->CreateSwapChainForHwnd(app.device, hwnd, &swapchain_desc, &swapchain_fullscreen_desc, nil, &app.swapchain)
	if !win.SUCCEEDED(result) {
		fmt.printf("DXGI: Failed to create Swapchain %v\n", u32(result))
		return false
	}

	CreateSwapchainResources(app) or_return

	return true
}

@(private)
Cleanup :: proc(app : ^HelloTriangleApplication) {
	framework.Cleanup(app)
}

@(private)
Load :: proc (app : ^HelloTriangleApplication) -> (ok : b32) {
	return true
}

@(private)
Update :: proc (app : ^HelloTriangleApplication) {

}

@(private)
Render :: proc (app : ^HelloTriangleApplication) {
	viewport := d3d.VIEWPORT{
		TopLeftX = 0.0,
		TopLeftY = 0.0,
		Width = f32(app.dimensions.x),
		Height = f32(app.dimensions.y),
		MinDepth = 0.0,
		MaxDepth = 1.0,
	}

	clear_color := [?]f32{ 50.0 / 256.0, 125.0 / 256.0, 250.0 / 256.0, 1.0 }

	app.device_context->ClearRenderTargetView(app.render_target_view, &clear_color)
	app.device_context->RSSetViewports(1, &viewport)
	app.device_context->OMSetRenderTargets(1, &app.render_target_view, nil)
	app.swapchain->Present(1, 0)
}

@(private)
ResizeHandler :: proc (window : glfw.WindowHandle, width : i32, height : i32) {
	ptr := (^HelloTriangleApplication)(glfw.GetWindowUserPointer(window))
	OnResize(ptr, width, height)
}

@(private)
OnResize :: proc (app : ^HelloTriangleApplication, width : i32, height : i32) {
	app.dimensions = {width, height}
	app.device_context->Flush()

	DestroySwapchainResources(app)

	result := app.swapchain->ResizeBuffers(0, u32(app.dimensions.x), u32(app.dimensions.y), .UNKNOWN, 0)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to recreate Swapchain buffers %x\n", u32(result))
		return
	}

	CreateSwapchainResources(app)
}

@(private)
CreateSwapchainResources :: proc (app : ^HelloTriangleApplication) -> b32 {
	backbuffer : ^d3d.ITexture2D = nil
	result := app.swapchain->GetBuffer(0, d3d.ITexture2D_UUID, (^rawptr)(&backbuffer))
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to get Backbuffer from swapchain %v\n", u32(result))
		return false
	}
	defer backbuffer->Release()

	result = app.device->CreateRenderTargetView(backbuffer, nil, &app.render_target_view)
	if !win.SUCCEEDED(result) {
		fmt.printf("D3D11: Failed to create RTV from Backbuffer %v\n", u32(result))
		return false
	}

	return true
}

@(private)
DestroySwapchainResources :: proc (app : ^HelloTriangleApplication) {
	app.device_context->OMSetRenderTargets(0, nil, nil)
	if app.render_target_view != nil {
		app.render_target_view->Release()
		app.render_target_view = nil
	}
}