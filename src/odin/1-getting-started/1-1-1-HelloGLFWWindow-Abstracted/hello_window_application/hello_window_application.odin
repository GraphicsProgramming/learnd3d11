package hello_window_application

import "core:fmt"

import "../../../framework"

HelloWindowApplication :: struct {
	using _application : framework.Application,
}

CreateHelloWindowApplication :: proc (title : string) -> (app : HelloWindowApplication) {
	app.title = title
	app._application_vtable = framework.application_vtable
	app.Load = Load
	app.Update = Update
	app.Render = Render
	
	return
}

@(private)
Load :: proc (app : ^framework.Application) -> (ok : b32) {
	return true
}

@(private)
Update :: proc (app : ^framework.Application) {

}

@(private)
Render :: proc (app : ^framework.Application) {
	
}