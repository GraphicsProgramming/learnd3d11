# Abstractions and Quality of Life improvements

Before we move on, we are going to change a few things. When you write software you don't always want all your code
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

This file will contain all future vertex types as well.

## Pipeline & DeviceContext

### Pipeline

What is a pipeline? It's just an immutable container of various things. It describes all state for the current thing you want to render or compute. All Vertex and Pixel or Compute shaders we might require for that render task to complete, the type of primitives we want to draw and how they are set up. Since we are coming from the Hello Triangle chapter our pipeline will not contain much, but it will grow in complexity further down the chapter road.

### PipelineFactory

`PipelineFactory` will handle the creation of `Pipeline` for us, which includes loading and compiling shaders, figuring out the right input layout by the given vertex type, for now.

### DeviceContext

`DeviceContext` is an abstraction over DX's native ID3D11DeviceContext, which has plenty of methods you need to call in order to get your triangles on screen. `DeviceContext` will know what to call specifically and handles that for you to keep the actual business logic "clean".

### Migration to Pipeline/PipelineFactory/DeviceContext

Let's create a `Pipeline.hpp` and add the following

```cpp
#pragma once
#include "Definitions.hpp"

#include <d3d11_2.h>

#include <cstdint>

class Pipeline
{
public:
    friend class PipelineFactory;
    friend class DeviceContext;

    void SetViewport(
        float left,
        float top,
        float width,
        float height);

private:
    WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
    WRL::ComPtr<ID3D11InputLayout> _inputLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology = {};
    uint32_t _vertexSize = 0;
    D3D11_VIEWPORT _viewport = {};
};
```

`Pipeline` is supposed to be an immutable object, therefore all relevant fields are `private`, so that you can't accidentally set them from outside. Only `PipelineFactory` will be able to access those fields, as its creating them. Also `DeviceContext` can access them too, as it needs these to set the actual state/values.

We also create `Pipeline.cpp` with the following content:

```cpp
#include "Pipeline.hpp"

void Pipeline::SetViewport(
    const float left,
    const float top,
    const float width,
    const float height)
{
    _viewport.TopLeftX = left;
    _viewport.TopLeftY = top;
    _viewport.Width = width;
    _viewport.Height = height;
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;
}
```

Its quite empty for now.

Let's move on to `PipelineFactory`. Create a new file `PipelineFactory.hpp` and add the following content:

```cpp
#pragma once

#include "Definitions.hpp"
#include "VertexType.hpp"
#include "Pipeline.hpp"

#include <d3d11_2.h>

#include <unordered_map>
#include <string>
#include <memory>

struct PipelineDescriptor
{
    std::wstring VertexFilePath;
    std::wstring PixelFilePath;
    VertexType VertexType;
};

class PipelineFactory
{
public:
    PipelineFactory(const WRL::ComPtr<ID3D11Device>& device);

    bool CreatePipeline(
        const PipelineDescriptor& settings,
        std::unique_ptr<Pipeline>& pipeline);

private:
    static size_t GetLayoutByteSize(VertexType vertexType);

    [[nodiscard]] WRL::ComPtr<ID3D11VertexShader> CreateVertexShader(
        const std::wstring& filePath,
        WRL::ComPtr<ID3DBlob>& vertexShaderBlob) const;
    [[nodiscard]] WRL::ComPtr<ID3D11PixelShader> CreatePixelShader(const std::wstring& filePath) const;

    bool CreateInputLayout(
        VertexType layoutInfo,
        const WRL::ComPtr<ID3DBlob>& vertexBlob,
        WRL::ComPtr<ID3D11InputLayout>& inputLayout);

    bool CompileShader(
        const std::wstring& filePath,
        const std::string& entryPoint,
        const std::string& profile,
        WRL::ComPtr<ID3DBlob>& shaderBlob) const;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    std::unordered_map<VertexType, std::vector<D3D11_INPUT_ELEMENT_DESC>> _layoutMap;
};
```

You can remove the methods from HelloTriangleApplication.hpp and HelloTriangleApplication.cpp since we are now using a separate class.

We also need `PipelineFactory.cpp` with the following content.

```cpp
size_t PipelineFactory::GetLayoutByteSize(const VertexType vertexType)
{
    switch (vertexType)
    {
        case VertexType::PositionColor: return sizeof(VertexPositionColor);
    }
    return 0;
}

PipelineFactory::PipelineFactory(const WRL::ComPtr<ID3D11Device>& device)
{
    _device = device;

    _layoutMap[VertexType::PositionColor] =
    {
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                offsetof(VertexPositionColor, position),
                D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR",
                0,
                DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                offsetof(VertexPositionColor, color),
                D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
        }
    };
}

bool PipelineFactory::CreatePipeline(
    const PipelineDescriptor& settings,
    std::unique_ptr<Pipeline>& pipeline)
{
    WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    pipeline = std::make_unique<Pipeline>();
    pipeline->_vertexShader = CreateVertexShader(settings.VertexFilePath, vertexShaderBlob);
    pipeline->_pixelShader = CreatePixelShader(settings.PixelFilePath);
    if (!CreateInputLayout(settings.VertexType, vertexShaderBlob, pipeline->_inputLayout))
    {
        return false;
    }
    pipeline->_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pipeline->_vertexSize = static_cast<uint32_t>(GetLayoutByteSize(settings.VertexType));
    return true;
}

bool PipelineFactory::CompileShader(
    const std::wstring& filePath,
    const std::string& entryPoint,
    const std::string& profile,
    WRL::ComPtr<ID3DBlob>& shaderBlob) const
{
    constexpr uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    WRL::ComPtr<ID3DBlob> tempShaderBlob = nullptr;
    WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    if (FAILED(D3DCompileFromFile(
        filePath.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.data(),
        profile.data(),
        compileFlags,
        0,
        &tempShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Failed to read shader from file\n";
        if (errorBlob != nullptr)
        {
            std::cout << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        }

        return false;
    }

    shaderBlob = std::move(tempShaderBlob);
    return true;
}

WRL::ComPtr<ID3D11VertexShader> PipelineFactory::CreateVertexShader(
    const std::wstring& filePath,
    WRL::ComPtr<ID3DBlob>& vertexShaderBlob) const
{
    if (!CompileShader(filePath, "Main", "vs_5_0", vertexShaderBlob))
    {
        return nullptr;
    }

    WRL::ComPtr<ID3D11VertexShader> vertexShader;
    if (FAILED(_device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        std::cout << "D3D11: Failed to compile vertex shader\n";
        return nullptr;
    }

    return vertexShader;
}

WRL::ComPtr<ID3D11PixelShader> PipelineFactory::CreatePixelShader(const std::wstring& filePath) const
{
    WRL::ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
    if (!CompileShader(filePath, "Main", "ps_5_0", pixelShaderBlob))
    {
        return nullptr;
    }

    WRL::ComPtr<ID3D11PixelShader> pixelShader;
    if (FAILED(_device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader)))
    {
        std::cout << "D3D11: Failed to compile pixel shader\n";
        return nullptr;
    }

    return pixelShader;
}

bool PipelineFactory::CreateInputLayout(
    const VertexType layoutInfo,
    const WRL::ComPtr<ID3DBlob>& vertexBlob,
    WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
    const std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = _layoutMap[layoutInfo];
    if (FAILED(_device->CreateInputLayout(
        inputLayoutDesc.data(),
        static_cast<uint32_t>(inputLayoutDesc.size()),
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        &inputLayout)))
    {
        std::cout << "D3D11: Failed to create the input layout";
        return false;
    }
    return true;
}
```

Most of the methods are now private and cannot be called from outside `PipelineFactory` and that is good. There is no need for somebody or something else to
create arbitrary shaders or other pipeline relevant things.

On to `DeviceContext`. Create a new `DeviceContext.hpp` with the following content

```cpp
#pragma once

#include "Definitions.hpp"

#include <d3d11_2.h>

#include <cstdint>

class Pipeline;

class DeviceContext
{
public:
    DeviceContext(WRL::ComPtr<ID3D11DeviceContext>&& deviceContext);

    void Clear(
        ID3D11RenderTargetView* renderTarget,
        float clearColor[4]) const;
    void SetPipeline(const Pipeline* pipeline);
    void SetVertexBuffer(
        ID3D11Buffer* triangleVertices,
        uint32_t vertexOffset);
    void Draw() const;
    void Flush() const;

private:
    uint32_t _drawVertices;
    const Pipeline* _activePipeline;
    WRL::ComPtr<ID3D11DeviceContext> _deviceContext;
};
```

And its implementation file `DeviceContext.cpp` with the following content

```cpp
#include "DeviceContext.hpp"
#include "Pipeline.hpp"

#include <utility>

DeviceContext::DeviceContext(WRL::ComPtr<ID3D11DeviceContext>&& deviceContext)
{
    _deviceContext = std::move(deviceContext);
    _activePipeline = nullptr;
    _drawVertices = 0;
}

void DeviceContext::Clear(
    ID3D11RenderTargetView* renderTarget,
    float clearColor[4]) const
{
    _deviceContext->ClearRenderTargetView(renderTarget, clearColor);
    _deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
}

void DeviceContext::SetPipeline(const Pipeline* pipeline)
{
    _activePipeline = pipeline;
    _deviceContext->IASetInputLayout(pipeline->_inputLayout.Get());
    _deviceContext->IASetPrimitiveTopology(pipeline->_primitiveTopology);
    _deviceContext->VSSetShader(pipeline->_vertexShader.Get(), nullptr, 0);
    _deviceContext->PSSetShader(pipeline->_pixelShader.Get(), nullptr, 0);
    _deviceContext->RSSetViewports(1, &pipeline->_viewport);
}

void DeviceContext::SetVertexBuffer(
    ID3D11Buffer* triangleVertices,
    uint32_t vertexOffset)
{
    D3D11_BUFFER_DESC description = {};
    triangleVertices->GetDesc(&description);
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        &triangleVertices,
        &_activePipeline->_vertexSize,
        &vertexOffset);
    _drawVertices = description.ByteWidth / _activePipeline->_vertexSize;
}

void DeviceContext::Draw() const
{
    _deviceContext->Draw(_drawVertices, 0);
}

void DeviceContext::Flush() const
{
    _deviceContext->Flush();
}
```

As you can see, `DeviceContext` will make sure all the state is set properly.

[Project on GitHub](https://github.com/GraphicsProgramming/learnd3d11/tree/main/src/Cpp/1-getting-started/1-1-3-HelloTriangle-Refactored)

[Next chapter](../1-2-debug/1-2-0-overview.md)
