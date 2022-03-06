# Hello Window

!!! error "Provide Link to HelloEmpty project"

    Project contains all the libs necessary (GLFW,assimp,$IMAGE_LIB)
    Main.cpp only contains

    ```cpp
    int main(int argc, char* argv[])
    {
        return 0;
    }
    ```

    But can be compiled and runs

[Link to the full source code](https://github.com/GraphicsProgramming/learnd3d11/tree/main/src/Cpp/1-getting-started/1-1-1-HelloWindow)

Let's start with the whole example code for creating a simple window first. We use the GLFW library
to create our window and as you can probably see, it's really not that much code. The following code
opens a new blank window titled `"LearnD3D11 - Hello Window"` and will leave it open until you hit
close.

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

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
	const int32_t width = static_cast<int32_t>(videoMode->width * 0.9f);
	const int32_t height = static_cast<int32_t>(videoMode->height * 0.9f);

	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(
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

Now let's go over everything in that example in more detail.

```cpp
#include <GLFW/glfw3.h>
```

C++ needs to know where all the definitions and declarations are coming from. We therefore have
to include the GLFW header so that everything we need is present in our source file.

```cpp
if (!glfwInit())
{
    std::cout << "GLFW: Unable to initialize\n";
    return -1;
}
```

Pretty obvious, right? [glfwInit](https://www.glfw.org/docs/3.3/group__init.html#ga317aac130a235ab08c6db0834907d85e)
tries to initialize `GLFW`. If it fails to do so, let the user know and end the program, since
there is no point in going further.

```cpp
GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
const int32_t width = static_cast<int32_t>(videoMode->width * 0.9f);
const int32_t height = static_cast<int32_t>(videoMode->height * 0.9f);
```

This piece of code grabs the main monitor via [glfwGetPrimaryMonitor](https://www.glfw.org/docs/3.3/group__monitor.html#gac3adb24947eb709e1874028272e5dfc5)
and its current resolution with [glfwGetVideoMode](https://www.glfw.org/docs/3.3/group__monitor.html#gaba376fa7e76634b4788bddc505d6c9d5),
so that we can derive a window width and height from it - and it will look similar no matter what
resolution you use. Size-wise it will cover 90% of your main monitor.

```cpp
glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
```

This will tell `GLFW` to not scale the window in any way, should you have setup a specific scaling
other than 100% on your desktop. That will keep the windowsize at what we set it, without thinking
about odd fractionals to manually scale the windowsize for any arbitrary scaling on your OS.

```cpp
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
```

`GLFW` was initially meant to support development of OpenGL based applications, hence the gl in its
name, but over the years it also started to support other APIs and not just OpenGL. Now since `GLFW`
automatically creates a context for OpenGL and we want to use DirectX, we will have to tell it to not
to do so via [glfwWindowHint](https://www.glfw.org/docs/3.3/group__window.html#ga7d9c8c62384b1e2821c4dc48952d2033).

There are many other options one can define through glfwWindowHint which can be found [here](https://www.glfw.org/docs/3.3/group__window.html).
Many of these options might be useful in your application, depending on what you want and how you
want to design your window.

```cpp
GLFWwindow* window = glfwCreateWindow(
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
```

This piece actually creates the window, if everything goes well. We pass in desired window
dimensions and a title, and call [glfwCreateWindow](https://www.glfw.org/docs/3.3/group__window.html#ga3555a418df92ad53f917597fe2f64aeb).
Make sure to check the return value, window creation can fail.

```cpp
const int32_t windowLeft = videoMode->width / 2 - width / 2;
const int32_t windowTop = videoMode->height / 2 - height / 2;
glfwSetWindowPos(window, windowLeft, windowTop);
```

I like centered windows, but `GLFW` will not place the window in a centered fashion,
because of that we try to do it ourselves here with the help of a bit of math and [glfwSetWindowPos](https://www.glfw.org/docs/3.3/group__window.html#ga1abb6d690e8c88e0c8cd1751356dbca8). It sets the window position in screen coordinates, specified by the top left corner of the window.

```cpp
while (!glfwWindowShouldClose(window))
{
    glfwPollEvents();
    // future update code
    // future render code
}
```

That is more or less the heart of your application, the mainloop.
You could also call it game loop, since in here everything happens.
From reading keyboard and mouse input, reacting to it, to telling the graphics
card to put a frog on the screen. It will keep doing it, until it gets signaled
to not to do that anymore because you closed the window for example
([glfwWindowShouldClose](https://www.glfw.org/docs/3.3/group__window.html#ga24e02fbfefbb81fc45320989f8140ab5)),
or hit Escape and mapped Escape to close the window. [glfwPollEvents](https://www.glfw.org/docs/3.3/group__window.html#ga37bd57223967b4211d60ca1a0bf3c832)
will make sure that `GLFW` knows about all required events coming from the operating system.

```cpp
glfwDestroyWindow(window);
glfwTerminate();
return 0;
```

Now we clean up the resources we have created, such as the window itself and
the `GLFW` system. Then simply return to the OS, without any error.

[glfwDestroyWindow](https://www.glfw.org/docs/3.3/group__window.html#gacdf43e51376051d2c091662e9fe3d7b2)
will obviously destroy the window and [glfwTerminate](https://www.glfw.org/docs/3.3/group__init.html#gaaae48c0a18607ea4a4ba951d939f0901)
cleans up `GLFW`.

When you start the program, you should see something like this.

![](../../images/1-1-1-hello-window-1.png)

!!! note "Why GLFW?"
    
    We have decided to include GLFW to manage our window (and later input) 
    because it is a very simple library, ready to use and above all, because of 
    consistency, plenty of other resources use GLFW, so it should be easier to 
    transfer the knowledge you will gain here elsewhere.

    There are other libraries, which handle windowing and input (amongst other things) out there as well, here are a few to choose from:

    - [SDL2](https://www.libsdl.org)
    - [SFML](https://www.sfml-dev.org)

    You could also write all that using the native functions and constructs the OS provides, on windows it would be WinAPI, and although it is old and not necessary to do all the heavylifting yourself, a lot of Microsoft's own guides rely on the WinAPI so we [have an example](Link to Win32) on that. We will not be covering any other Win32 topics, unless it is explicitly required 

    You are probably going to continue writing your own engine after this tutorial and might decide to support more platforms, other than just windows, then this is already covered by `GLFW`.
   

## First abstraction

The further we go into this tutorial series the more stuff we will add to the program.
But we don't want to cram everything into `Main.cpp`, your main entry point of the program.
A good practise is to split up things into smaller units, to not lose overview.

Right now we dont have much to show for, just a window, made by a few lines of code,
but we are going to abstract that code into a new class called `Application` which will also be
our main container so to speak, in which all the magic will happen.

I will show the whole code first, and then explain again what means what.

### Application.hpp

```cpp
#pragma once

#include <string_view>

struct GLFWwindow;

class Application
{
public:
    Application(const std::string_view title);
    virtual ~Application();
    void Run();

protected:
    virtual void Cleanup();
    virtual bool Initialize();
    virtual void Render() = 0;
    virtual void Update() = 0;

private:
    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string_view _title;
};
```

### Application.cpp

```cpp
#include "Application.hpp"
#include <GLFW/glfw3.h>

Application::Application(const std::string_view title)
{
    _title = title;
}

Application::~Application()
{
    Cleanup();
}

void Application::Run()
{
    if (!Initialize())
    {
        return;
    }

    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
        Update();
        Render();
    }
}

void Application::Cleanup()
{
    if (_window != nullptr)
    {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }
    glfwTerminate();
}

void Application::Close()
{
    glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

bool Application::Initialize()
{
    if (!glfwInit())
    {
        std::cout << "GLFW: Unable to initialize\n";
        return false;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    _width = static_cast<int32_t>(videoMode->width * 0.9f);
    _height = static_cast<int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
    if (_window == nullptr)
    {
        std::cout << "GLFW: Unable to create window\n";
        return false;
    }

    const int32_t windowLeft = videoMode->width / 2 - _width / 2;
    const int32_t windowTop = videoMode->height / 2 - _height / 2;
    glfwSetWindowPos(_window, windowLeft, windowTop);

    return true;
}
```

## HelloWindowApplication.hpp

```cpp
#include "Application.hpp"

class HelloWindowApplication final : public Application
{
public:
    HelloWindowApplication(const std::string_view title);

protected:
    void Render() override;
    void Update() override;
}
```

## HelloWindowApplication.cpp

```cpp
HelloWindowApplication::HelloWindowApplication(const std::string_view title)
    : Application(title)
{
}

void HelloWindow::Update()
{
}

void HelloWindow::Render()
{
}
```

### Main.cpp

```cpp
int main(int argc, char* argv[])
{
    HelloWindowApplication application;
    application.Run();
}
```

Let's start with `Main.cpp`. That's all its doing, creating the hellowindow application and running it.
In the future this can be accompanied by loading a configuration, initializing a logger,
initiating a connection to a possible server server, or other stuff.

```cpp
public:
...
    void Run();
...
```

This is a section of the `Application` class, showing only its publicly available methods. `Run` being
the most important one to the outside world, like `Main`, it's the entry point into this Application.

We still dont want to cram everything into one main or one method, therefore `Run` is split up
again into the following blocks.

```cpp
void Application::Run()
{
    if (!Initialize())
    {
        return;
    }

    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
        Update();
        Render();
    }
}
```

You can clearly see what it is doing. `Initialize`, as the name suggests, will initialize everything
which is required for the app to run, which currently is the window in our case.

The next block is the aforementioned mainloop or gameloop, which still does what it was doing before,
checking with the OS if events need to be processed and now we also call a `Update` and `Render` method.

`Update` may contain queries about pressed key or mouse buttons, updating variables or other things
which are - for instance - reflected on display inside the `Render` method.

You probably have noticed that all the protected method in `Application` are virtual, that's because
we are deriving from `Application` in form of `HelloWindowApplication` and only focus on those four
methods if required. We now dont have to deal with the mainloop anymore for the time being.

```cpp
virtual void Cleanup();
virtual bool Initialize();
virtual void Render() = 0;
virtual void Update() = 0;
```

If you run this example, you will still get the same window as shown below, same behaviour, only the code has been
spit up into a more logical piece of work, which will make our life easier as we move on adding more
and more.

![](../../images/1-1-1-hello-window-1.png)

[Next chapter](../1-1-2-hello-d3d11/)
