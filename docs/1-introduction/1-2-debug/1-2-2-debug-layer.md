# Debug Layer

During development it is important to have tools to help you. Be it pointing out typos, obvious error
or simple hints why things might not draw on screen.

D3D11 provides a debug layer, which can give you hints, warnings and errors when you put in the wrong
values into calls for d3d11 functions.

This debug layer needs to be actively enabled, which is a simple flag you have to set when creating
the device.

Right now we have thee `deviceFlags`

```cpp
UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;
```

and change it to

```cpp
    UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif
```

Obviously this makes sense when you are running this in the `Debug` configuration where the preprocessor variable `DEBUG`/`_DEBUG` is defined,
or rather where when is `NDEBUG` is not defined :) (like it is in `Release` configuration).

in `DebugLayer.hpp` we have to add the following member variable

```cpp
ComPtr<ID3D11Debug> _debug = nullptr;
```

and we initialize it in `DebugLayer.cpp`'s `Initialize`, right after we create device and device context.

```cpp
if (FAILED(_device.As(&_debug)))
{
    std::cout << "D3D11: Failed to get the debug layer from the device\n";
    return false;
}
```

and we also adjust the destructor too

```cpp
    _deviceContext.Reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
```

`ReportLiveDeviceObjects` even tells us at the end of the application which and how many objects are still alive from our d3d11 adventure.

It will help us track leaks in the future.

Now try changing the following value to see the actual debug layer in action.

Find

```cpp
swapChainDescriptor.BufferCount = 2;
```

and change it to

```cpp
swapChainDescriptor.BufferCount = 1;
```

The debug layer will yell at you, in the Console window of Visual Studio:

```bash
DXGI ERROR: IDXGIFactory::CreateSwapChain: Flip model swapchains (DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL and DXGI_SWAP_EFFECT_FLIP_DISCARD)
require BufferCount to be between 2 and DXGI_MAX_SWAP_CHAIN_BUFFERS, inclusively. DXGI_SWAP_CHAIN_DESC{ SwapChainType = ..._HWND,
BufferDesc = DXGI_MODE_DESC1{Width = 3456, Height = 1944, RefreshRate = DXGI_RATIONAL{ Numerator = 0, Denominator = 0 },
Format = B8G8R8A8_UNORM, ScanlineOrde
```

or find

```cpp
_deviceContext->Draw(3, 0);
```

and change it to

```cpp
_deviceContext->Draw(6, 0);
```

and the debug layer will tell you thats not cool (you cant draw more triangles than there are defined in the vertexbuffer/bound to the input assembly)

```bash
D3D11 WARNING: ID3D11DeviceContext::Draw: Vertex Buffer at the input vertex slot 0 is not big enough for what
the Draw*() call expects to traverse. This is OK, as reading off the end of the Buffer is defined to return 0.
However the developer probably did not intend to make use of this behavior.  [ EXECUTION WARNING #356: DEVICE_DRAW_VERTEX_BUFFER_TOO_SMALL]
```

!!! error "Explain InfoQueue"

    How to mute certain messages

!!! error "Explain InfoQueue via dxcpl"

    How to do that via the control panel

One thing which should go without saying, the debug layer will slow down your application a bit.
