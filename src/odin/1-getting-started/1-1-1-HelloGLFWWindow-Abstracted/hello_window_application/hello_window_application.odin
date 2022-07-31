package hello_window_application

import "../../../framework"

import "core:fmt"

B :: struct {
	using _aa : framework.A,
}
Two :: proc (a: ^framework.A) -> (ok : b32) {
	fmt.println("in two")
	return true
}
CreateB :: proc (num : i32) -> (result : B) {
	using result
	_aa.my = num
	_aa.one = framework.One
	_aa.two = Two
	return
}





HelloWindowApplication :: struct {
	using _application : framework.Application,
}

CreateHelloWindowApplication :: proc (title : string) -> (app : HelloWindowApplication) {
	using app
	_application.title = title
	// _application._application_vtable = framework.application_vtable
	_application._application_vtable.Run = framework.Run
	_application._application_vtable.Cleanup = framework.Cleanup
	_application._application_vtable.Initialize = framework.Initialize
	_application._application_vtable.Load = Load
	_application._application_vtable.Update = Update
	_application._application_vtable.Render = Render
	return
}

Load :: proc (app : ^framework.Application) -> (ok : b32) {
	fmt.println("test")
	return true
}

Update :: proc (app : ^framework.Application) {

}

Render :: proc (app : ^framework.Application) {
	
}