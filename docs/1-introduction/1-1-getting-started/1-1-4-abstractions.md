# Abstractions and Quality of Life improvements

Before we move on, we are going to change a few things. When you write software you dont always want all your code
in one spot, one method or one class, but split it up and separate where it makes sense to do so.

This is always a good exercise in projects which are getting a bit bigger, and this project will be getting
bigger, since we have to cover a lot of things a lot of components will be involved.

From now onwards we will explain everything you need to know to move forward in the usual way.
What needs to be added and where, along with the explanations. We will also guide you what you have to add on top of all that.

In the end we will have a cleaner structure of the whole thing, while still maintaining a way to find your self
around within the project.

Lets start:

## ComPtr

You might remember this

```cpp
class HelloTriangleApplication final : public Application
{
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
```

We will move this type alias into a separate header, so that other components which rely on ComPtr (and
make use of the shorter typename).

Create a new file in the project and call it `Definitions.hpp`.

It should look like this

```cpp
#pragma once

#include <wrl/client.h>

namespace WRL = Microsoft::WRL;
```

And make sure you include it. Then rename the `ComPtr<>` locations from

```cpp
ComPtr<ID3D11Device> _device = nullptr;
```

to

```cpp
WRL::ComPtr<ID3D11Device> _device = nullptr;
```

and all the others accordingly.

## VertexType

Next step, create a new file called `VertexType.hpp` and make sure to include it in `HelloTriangleApplication.cpp`

Then move

```cpp
using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;

struct VertexPositionColor
{
    Position position;
    Color color;
};
```

from `HelloTriangleApplication.cpp`
into the new `VertexType.hpp`

Should look like

```cpp
#pragma once

#include <DirectXMath.h>

enum class VertexType
{
    PositionColor
};

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;

struct VertexPositionColor
{
    Position position;
    Color color;
};
```

## Pipeline & DeviceContext

!!! error "Explain Pipeline"

!!! error "Explain PipelineFactory"

!!! error "Explain DeviceContext"

!!! error "Migrate from current hello-triangle to hello-triangle-refactored"
