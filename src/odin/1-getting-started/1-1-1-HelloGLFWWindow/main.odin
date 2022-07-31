package main

import "core:fmt"
import "vendor:glfw"

main :: proc () {
	result := glfw.Init()
	if result == 0 {
		fmt.println("GLFW: Unable to initialize")
		glfw.Terminate()
		return
	}

	primary_monitor := glfw.GetPrimaryMonitor()
	video_mode := glfw.GetVideoMode(primary_monitor)
	window_based_width := i32(f32(video_mode.width) * 0.9)
	window_based_height := i32(f32(video_mode.height) * 0.9)

	glfw.WindowHint(glfw.SCALE_TO_MONITOR, 0)
	glfw.WindowHint(glfw.CLIENT_API, glfw.NO_API)
	window := glfw.CreateWindow(window_based_width, window_based_height, "Learn D3D11 - Hello Window", nil, nil)
	if window == nil {
		fmt.println("GLFW: Unable to create window")
		glfw.Terminate()
		return
	}

	window_left := video_mode.width / 2 - window_based_width / 2
	window_top := video_mode.height / 2 - window_based_height / 2
	glfw.SetWindowPos(window, window_left, window_top)

	for !glfw.WindowShouldClose(window) {
		glfw.PollEvents()
		// future update code
		// future render code
	}

	glfw.DestroyWindow(window)
	glfw.Terminate()
	return
}