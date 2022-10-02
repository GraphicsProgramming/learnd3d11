package hello_triangle_application

import "core:fmt"
import win "core:sys/windows"
import "vendor:glfw"
import d3d "vendor:directx/d3d11"
import "vendor:directx/dxgi"
import d3dc "vendor:directx/d3d_compiler"

import "../../../framework"
import "../renderer/d3d11"

ImageLibraryApplication :: struct {
	using renderer : d3d11.Renderer,
	using application : framework.Application,
}

CreateImageLibraryApplication :: proc (title : string) -> (app : ImageLibraryApplication) {
	app.title = title
	return
}

Run :: proc (app : ^ImageLibraryApplication) {
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
		d3d11.Render(&app.application, &app.renderer)
	}
}

@(private)
Initialize :: proc (app : ^ImageLibraryApplication) -> (ok : b32) {
	framework.Initialize(app) or_return

	glfw.SetWindowUserPointer(app.window, rawptr(app))
	glfw.SetFramebufferSizeCallback(app.window, glfw.FramebufferSizeProc(ResizeHandler))

	d3d11.Initialize(&app.application, &app.renderer) or_return

	return true
}

@(private)
Cleanup :: proc(app : ^ImageLibraryApplication) {
	framework.Cleanup(app)
}

@(private)
Load :: proc (app : ^ImageLibraryApplication) -> (ok : b32) {
	d3d11.Load(&app.application, &app.renderer) or_return
	
	return true
}

@(private)
Update :: proc (app : ^ImageLibraryApplication) {
	// framework.Update(&app.application)
	d3d11.Update(&app.application, &app.renderer)
}

@(private)
ResizeHandler :: proc (window : glfw.WindowHandle, width : i32, height : i32) {
	ptr := (^ImageLibraryApplication)(glfw.GetWindowUserPointer(window))
	d3d11.OnResize(&ptr.application, &ptr.renderer, u32(width), u32(height))
}