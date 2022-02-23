# Welcome to Learning Direct3D11

## Introduction

Goal of this thing

### DX API Overview

DirectX consists of various components such as

- `Direct3D11` elaborate...

- `Direct2D1` elaborate...

- `D3DCompiler` elaborate...

- `XAudio2` elaborate...

- `XACT` elaborate...

- `DirectWrite` elaborate...

- `XInput` elaborate...

but we are going to focus on mainly `Direct3D11`

### Direct3D11 Usage

Direct3D11 comes with quite a bit of classes and constructs to make use of the API.

- `ID3D11Device` elaborate...

- `ID3D11DeviceContext` elaborate...

- `ID3D11Texture` elaborate...

- `ID3D11ShaderResourceView` elaborate...

- `ID3D11UnorderedAccessViw` elaborate...

- `ID3D11PixelShader` elaborate...

- `ID3D11VertexShader` elaborate...

- `ID3D11InputLayout` elaborate...

- `ID3D11RasterizerState` elaborate... asd

### Libraries & Project Layout

For the LearnD3D11 tutorial series we will be using the following third party libraries

- [GLFW](https://www.glfw.org) It provides the window to render our things in, as well as input handling via mouse and keyboard. Yes there are also alternatives like `SDL2` for instance. It serves the same purpose, we just decided to go with `GLFW`. You might also wonder about "I want to do it myself, from scratch" then you really could do it yourself; but its the year 2022 by the time we are writing this thing and the problem of creating a window and handling input has been solved many times already, why reinvent the wheel. We have a link [here](DIYWinApi.md) to show how you could do it).

- [DirectXMath](...link...) As part of the Windows SDK will be the math library we use

- [Assimp](www.assimp.org) Assimp provides facilities to load mesh files of various file formats

- [dear IMGUI](https://github.com/ocornut/imgui) dear IMGUI is an easy to use immediate mode graphics user interface library, its widely used in the industry and we will be using it to display interactive options

#### Project Layout

- `.github` contains github related files to enable this project's existence
- `docs` contains this very documentation describing each chapter
- `lib` contains all external/third party libraries like `GLFW` or `AssImp`
- `src` contains all the example source code behind this tutorial series

# 0. Initial Setup

```todo
either clone repo, open empty solution with project and empty main.cpp
or explain how to create a project in VS, set it up so that it makes sense,
explain include/lib folders, how to set output folder and why Win32 is not
an option really anymore either
```

Assume we cloned the empty main, already setup with GLFW

Ok let's start with the whole example for a HelloWindow first

```cpp
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>

int main(int argc, char* argv[])
{
    if (!glfwInit())
    {
        std::cout << "GLFW: Unable to initialize\n";
        return -1;
    }

    const GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    const int32_t width = static_cast<int32_t>(videoMode->width * 0.9f);
    const int32_t height = static_cast<int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    const GLFWwindow* window = glfwCreateWindow(
        width,
        height,
        "LearnD3D11 - Hello Window",
        nullptr,
        nullptr);
    if (window == nullptr)
    {
        std::cout << "GLFW: Unable to create window\n";
        glfwTerminate();
        return -1;
    }

    const int32_t windowLeft = videoMode->width / 2 - width / 2;
    const int32_t windowTop = videoMode->height / 2 - height / 2;
    glfwSetWindowPos(window, windowLeft, windowTop);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // future update code
        // future render code
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

Now let me explain what all these things mean.

```cpp
#include <GLFW/glfw3.h>
```

C++ needs to know where all the definitions and declarations are coming from.

```cpp
    if (!glfwInit())
    {
        std::cout << "GLFW: Unable to initialize\n";
        return -1;
    }
```

Pretty obvious, right? Try to initialize `GLFW`. If it fails to do so, let the user know and end the program, since there is no point in going further.

```cpp
    const GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    const int32_t width = static_cast<int32_t>(videoMode->width * 0.9f);
    const int32_t height = static_cast<int32_t>(videoMode->height * 0.9f);
```

This piece of code grabs the main monitor's resolution, so that we can derive a window width and height from it - and it will look similar no matter what resolution you use.

```cpp
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
```

This will tell `GLFW` to not scale the window in any way, should you have setup a specific scaling other than 100% on your desktop. That will keep the windowsize at what we set it, without thinking about odd fractionals to manually scale the windowsize for any arbitrary scaling on your OS.

```cpp
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
```

`GLFW` was initially meant to support development of OpenGL based applications, hence the gl in its name, but over the years it also supports other APIs not just OpenGL. Now since `GLFW` automatically create a context for OpenGL we can tell it not to do it via that window hint.

```cpp
    const int32_t windowLeft = videoMode->width / 2 - width / 2;
    const int32_t windowTop = videoMode->height / 2 - height / 2;
    glfwSetWindowPos(window, windowLeft, windowTop);
```

Should be fairly obvious. We like centered windows.

```cpp
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // future update code
        // future render code
    }
```

That is more or less the heart of your application, the mainloop. You could also call it game loop, since in here everything happens. From reading keyboard and mouse input, reacting to it, to telling the graphics card to put a frog on the screen. It will also keep doing it, until it gets signaled to not to do that anymore because you closed the window for example, or hit Escape and mapped Escape to close the window.

```cpp
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
```

Now we clean up any resource we created, such as the window itself and the GLFW system. Then simply return to the OS, without any error.

When you start the program, you should see something like this

!!! error "Abstract current main into Application.cpp, explain Initialize/Update/Render/Cleanup

!!! error "Add image to show HelloWindow in action"

!!! error "Add Link to next chapter here"

# 1. Hello Triangle

## Initialize Direct3D

!!! error "Show whole D3D11 code with Application"

!!! error "Then walk through the code again"

## Abstraction into Application & D3D11Context

# 2. Graphics Pipeline

!!! error "explain in an overview fashion with pics what directx pipeline is, how it roughly works and what it means and can do"

![basically this](https://docs.microsoft.com/en-us/windows/win32/direct3d11/images/d3d11-pipeline-stages.jpg)
