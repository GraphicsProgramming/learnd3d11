package main

import app "image_library_application"

main :: proc () {
	image_library_application := app.CreateImageLibraryApplication("Learn D3D11 - Image Library")
	app.Run(&image_library_application)
}