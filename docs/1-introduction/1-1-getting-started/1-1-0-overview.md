# Overview

As many may know or not know, there's a few choices in terms of what to use for graphics, such as OpenGL, Vulkan, Metal and what is commonly known as "DirectX".
However in word of mouth DirectX usually refers to Direct3D, which is one of the many API's that DirectX has, as it is less known that DirectX is more than just graphics, it is:

- **Audio**

- **Fonts**

- **Input**

- **Graphics**

And _other things_ more or less targeted at stuff one needs for game development.

The goal of LearnD3D11 is as one might guess is the targeted explanation and showcase of Direct3D11 or as one generally refers to it: "DirectX 11".

During each step we'll provide a project for you to follow along as we explain each individual thing needed to get some graphics on your own screen.

This initial section will cover creating the actual window, initializing Direct3D11 and getting our very first visuals (which is commonly known as the Hello Triangle)

## Project structure

A few words about how each project will look like.

Inside the project folder:

```
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
