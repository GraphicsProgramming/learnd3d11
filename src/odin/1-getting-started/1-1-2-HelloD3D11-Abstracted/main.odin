package main

import "core:fmt"

import app "hello_d3d11_application"

main :: proc () {
	hello_d3d11_application := app.CreateHelloD3D11Application("Learn D3D11 - Hello D3D11 Abstracted")
	app.Run(&hello_d3d11_application)
}