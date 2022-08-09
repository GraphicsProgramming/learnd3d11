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
Initialize :: proc (app : ^HelloWindowApplication) -> (ok : b32) {
	framework.Initialize(app) or_return

	return true
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

@(private)
Cleanup :: proc (app : ^HelloWindowApplication) {
	framework.Cleanup(app)
}