package hello_window_application

import "core:fmt"
import "vendor:glfw"

import "../../../framework"

HelloWindowApplication :: struct {
	using _application : framework.Application,
}

CreateHelloWindowApplication :: proc (title : string) -> (app : HelloWindowApplication) {
	app.title = title
	return
}

Run :: proc (app : ^HelloWindowApplication) {
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
Load :: proc (app : ^HelloWindowApplication) -> (ok : b32) {
	return true
}

@(private)
Update :: proc (app : ^HelloWindowApplication) {

}

@(private)
Render :: proc (app : ^HelloWindowApplication) {
	
}