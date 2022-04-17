# Naming things

The debug layer might tell us possible leaks of various d3d11 related objects, the cool thing is,
it can tell you exactly which object is leaking, via its name.

We can give our d3d11 objects a name.

Each D3D11 object (not the device itself) derives from `ID3D11DeviceChild` and that interface
implements a method `SetPrivateData` which we can use to assign a name to all those objects.

For that we will introduce a macro (hopefully the only one :))

```cpp
template<UINT TDebugNameLength>
inline void SetDebugName(
    _In_ ID3D11DeviceChild* deviceResource,
    _In_z_ const char(&debugName)[TDebugNameLength])
{
    deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
}
```

And we use it like

```cpp
SetDebugName(_deviceContext.Get(), "CTX_Main");
```

Unfortunately not every object is/implements `ID3D11DeviceChild` so for the other things like
the dxgi factory or the device itself we have to use `SetPrivateData` the ordinary way

```cpp
constexpr char factoryName[] = "Factory1";
_factory->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(factoryName), factoryName);

constexpr char deviceName[] = "DEV_Main";
_device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
```

Now run the thing again and take a look at the summary report of the debug device in the output window

```bash
D3D11 WARNING: Live ID3D11Device at 0x000002214E8A1B10, Name: DEV_Main, Refcount: 3 [ STATE_CREATION WARNING #441: LIVE_DEVICE]
D3D11 WARNING:  Live ID3D11Context at 0x000002214E8A5B50, Name: CTX_Main, Refcount: 0, IntRef: 1 [ STATE_CREATION WARNING #2097226: LIVE_CONTEXT]
```

Notice anything?

Exactly, they show names.

[Project on GitHub](https://github.com/GraphicsProgramming/learnd3d11/tree/main/src/Cpp/1-getting-started/1-2-3-NamingThings)

[Next chapter](./1-2-4-renderdoc.md)
