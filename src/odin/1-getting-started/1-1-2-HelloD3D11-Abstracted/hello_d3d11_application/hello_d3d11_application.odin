package hello_window_application

import "core:fmt"
import "vendor:glfw"

import "../../../framework"

HelloD3D11Application :: struct {
	using _application : framework.Application,
}

CreateHelloD3D11Application :: proc (title : string) -> (app : HelloD3D11Application) {
	app.title = title
	return
}

Run :: proc (app : ^HelloD3D11Application) {
	ok := framework.Initialize(app)
	defer framework.Cleanup(app)
	if !ok {
		return
	} 

	if !Load(app) {
		return
	}

	for !glfw.WindowShouldClose(app.window) {
		glfw.PollEvents()
		Update(app)
		Render(app)
	}
}

@(private)
Load :: proc (app : ^HelloD3D11Application) -> (ok : b32) {
	return true
}

@(private)
Update :: proc (app : ^HelloD3D11Application) {

}

@(private)
Render :: proc (app : ^HelloD3D11Application) {
	
}