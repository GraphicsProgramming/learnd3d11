# Overview

As you may or may not know, there's a few choices in terms of what you can use for rendering, such
as OpenGL, Vulkan, Metal, and what is commonly known as "DirectX". However, in word of mouth
DirectX usually refers to Direct3D, which is one of the many API's that DirectX has. In fact
DirectX provides tooling and libraries for more aspects of game development, including:

- **Audio**
- **Fonts**
- **Input**
- **Graphics**

The goal of LearnD3D11 is, as one might guess, the targeted explanation and showcase of Direct3D11
within C++, or as one generally refers to it: "DirectX 11". This guide does _NOT_ cover DirectX 12,
the newer version of the API, as it is very low level and will usually require a good understanding
of the GPU and how it works. We will cover concepts and some techniques used in graphics programming,
and we do not expect you to have any prerequisites in this field. It is, however, not a guide
covering C++ or programming in general; we expect at least the ability and understanding to write
object-oriented programs in C++.

During each step we'll provide a project for you to follow along as we explain everything for you
to start rendering geometry using your GPU.

Also note that DirectX is made by Microsoft and is generally only available on Windows. However,
`DXVK` was developed to run D3D9 through D3D11 on Linux or Wine on top of Vulkan and would be the
only way of developing and using D3D11 on those platforms. 

This initial section will cover creating the actual window, initializing Direct3D11 and getting our
very first visuals (which is commonly known as the Hello Triangle)

## Project structure

A few words about how each project will look like.

Inside the project folder:

```bash
Assets/
Assets/Models/
Assets/Shaders/
Assets/Textures/
bin/Debug/
bin/Release/
obj/Debug/
obj/Release/
Main.cpp
x-x-x-project.vcxproj.filters
x-x-x-project.vcxproj.user
x-x-x-project.vcxproj
```

- The `vcxproj` files are part of Visual Studio's project system.
- `Main.cpp` is the entry point of each application. (in the first few chapters we will have all the code in there, but later refactor them out into their own units)
- `obj/` contains all intermediate junk the compiler produced, to keep the folder structure clean
- `bin/` will contain the compiled program of the chapter along with all necessary `Assets`
- `Assets/` will contain all the used assets, such as models, shaders and textures and other things. It will be empty for the first few chapters, and we will copy it and its contents to the bin/Debug or bin/Release directory, depending on which configuration you chose
