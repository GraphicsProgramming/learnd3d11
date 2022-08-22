# Instructions and Tips for odin
odin's compiler can be installed by following the instructions here.
https://odin-lang.org/docs/install/

odin provides bindings for glfw and directx libraries through its vendor library collection.
https://pkg.odin-lang.org/vendor/

## Building and Running
Installing odin and building the compiler should add it to the PATH as `odin`. To build one of the lessons go into the lessons directory and run one of the following commands:

`odin build .` to build.

`odin run .` to build and run.

## No ComPtr
The learnd3d11 tutorial makes heavy use of ComPtr, but odin does not have bindings for this functionality.
As a result, the developer will need to do some additional bookkeeping with certain elements of dxgi and d3d.
The first place this is necessary is 1-1-3. When `CreateSwapchainResources` is called, the backbuffer it creates
must be `Release`-d before the function exits, otherwise resizing the window will fail and crash the program.


## TODO
[] Odin currently doesn't have a binding for DirectXTex, nor an image loader for .dds files.
	[] Implement bindings / Wait for someone else to develop and publish bindings
	[] Use DirectXTex bindings, or continue with whatever was used in the meantime.