package main

import "core:fmt"

import app "hello_window_application"

import "../../framework"





main :: proc () {
	//b : app.B
	//b._a.one = framework.One
	//b._a.two = app.Two
	//b->one()

	c := app.CreateB(1)
	c->one()

	/*
	application : framework.Application
	application.title = "test"
	application._application_vtable = framework.application_vtable
	application._application_vtable.Load = proc (app : ^framework.Application) -> b32 {
		return true
	}
	application._application_vtable.Update = proc (app : ^framework.Application) {

	}
	application._application_vtable.Render = proc (app : ^framework.Application) {
		
	}
	application->Run()
	*/
	
	//hello_window_application := app.CreateHelloWindowApplication("Learn D3D11 - Hello Window Abstracted")
	//hello_window_application->Run()
}