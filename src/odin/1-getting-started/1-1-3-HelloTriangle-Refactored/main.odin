package main

import "core:fmt"

import app "hello_triangle_application"

main :: proc () {
	hello_triangle_application := app.CreateHelloTriangleApplication("Learn D3D11 - Hello Triangle Refactored")
	app.Run(&hello_triangle_application)
}