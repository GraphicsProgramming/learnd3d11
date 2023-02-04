#include "RasterizerStateApplication.hpp"
#include "ShaderCollection.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3dcompiler.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

template <UINT TDebugNameLength>
inline void SetDebugName(_In_ ID3D11DeviceChild* deviceResource, _In_z_ const char (&debugName)[TDebugNameLength])
{
    deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
}

RasterizerStateApplication::RasterizerStateApplication(const std::string& title)
    : Application(title)
{
}

RasterizerStateApplication::~RasterizerStateApplication()
{
    _deviceContext->Flush();

    _wireFrameCullBackRasterizerState.Reset();
    _wireFrameCullFrontRasterizerState.Reset();
    _wireFrameCullNoneRasterizerState.Reset();

    _solidFrameCullBackRasterizerState.Reset();
    _solidFrameCullFrontRasterizerState.Reset();
    _solidFrameCullNoneRasterizerState.Reset();

    _shaderCollection.Destroy();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
}

bool RasterizerStateApplication::Initialize()
{
    // This section initializes GLFW and creates a Window
    if (!Application::Initialize())
    {
        return false;
    }

    // This section initializes DirectX's devices and SwapChain
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
    {
        std::cout << "DXGI: Failed to create factory\n";
        return false;
    }

    constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
    uint32_t deviceFlags = 0;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

    WRL::ComPtr<ID3D11DeviceContext> deviceContext = nullptr;
    if (FAILED(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            deviceFlags,
            &deviceFeatureLevel,
            1,
            D3D11_SDK_VERSION,
            &_device,
            nullptr,
            &deviceContext)))
    {
        std::cout << "D3D11: Failed to create device and device context\n";
        return false;
    }

    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }


    constexpr char deviceName[] = "DEV_Main";
    _device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(deviceContext.Get(), "CTX_Main");

    _deviceContext = deviceContext;

    DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
    swapChainDescriptor.Width = GetWindowWidth();
    swapChainDescriptor.Height = GetWindowHeight();
    swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDescriptor.SampleDesc.Count = 1;
    swapChainDescriptor.SampleDesc.Quality = 0;
    swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescriptor.BufferCount = 2;
    swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDescriptor.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapChainDescriptor.Flags = {};

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
    swapChainFullscreenDescriptor.Windowed = true;

    if (FAILED(_dxgiFactory->CreateSwapChainForHwnd(
            _device.Get(),
            glfwGetWin32Window(GetWindow()),
            &swapChainDescriptor,
            &swapChainFullscreenDescriptor,
            nullptr,
            &_swapChain)))
    {
        std::cout << "DXGI: Failed to create swapchain\n";
        return false;
    }

    CreateSwapchainResources();

    return true;
}

bool RasterizerStateApplication::Load()
{
    ShaderCollectionDescriptor shaderDescriptor = {};
    shaderDescriptor.VertexShaderFilePath = L"Assets/Shaders/Main.vs.hlsl";
    shaderDescriptor.PixelShaderFilePath = L"Assets/Shaders/Main.ps.hlsl";
    shaderDescriptor.VertexType = VertexType::PositionColor;

    _shaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

    D3D11_BUFFER_DESC triangleBufferDesc{};
    triangleBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    triangleBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    triangleBufferDesc.ByteWidth = sizeof(VertexPositionColor) * 3;
    float xPos = -1.0f;
    float yPos = 1.0f;
    for (int i = 0; i < _triangleVertices.size(); i++)
    {
        VertexPositionColor vertices[] = {
            VertexPositionColor{Position{ xPos + 0.5f, yPos - 0.25f, 1.0f }, Color{ 1.0f, 0.0f, 0.0f }},
            VertexPositionColor{Position{ xPos + 0.33f, yPos - 0.5f, 1.0f }, Color{ 0.0f, 1.0f, 0.0f }},
            VertexPositionColor{Position{ xPos + 0.66f, yPos - 0.5f, 1.0f }, Color{ 0.0f, 0.0f, 1.0f }},
        };

        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = vertices;
        sd.SysMemPitch = sizeof(vertices) * 3;
        _device->CreateBuffer(&triangleBufferDesc, &sd, &_triangleVertices[i]);

        xPos += 0.5f;
        if (i == 2)
        {
            yPos -= 1.0f;
            xPos = -1.0f;
        }
    }

    if (!CreateRasterizerStates())
    {
        return false;
    }

    return true;
}

bool RasterizerStateApplication::CreateSwapchainResources()
{
    WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    if (FAILED(_swapChain->GetBuffer(
            0,
            IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get back buffer from the swapchain\n";
        return false;
    }

    if (FAILED(_device->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            &_renderTarget)))
    {
        std::cout << "D3D11: Failed to create rendertarget view from back buffer\n";
        return false;
    }

    return true;
}

void RasterizerStateApplication::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void RasterizerStateApplication::OnResize(
    const int32_t width,
    const int32_t height)
{
    Application::OnResize(width, height);
    _deviceContext->Flush();

    DestroySwapchainResources();

    if (FAILED(_swapChain->ResizeBuffers(
            0,
            width,
            height,
            DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,
            0)))
    {
        std::cout << "D3D11: Failed to recreate swapchain buffers\n";
        return;
    }

    CreateSwapchainResources();
}

void RasterizerStateApplication::Update()
{
    Application::Update();
}

void RasterizerStateApplication::Render()
{
    float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    ID3D11RenderTargetView* nullRTV = nullptr;
    constexpr uint32_t vertexOffset = 0;

    D3D11_VIEWPORT viewport = {
        0.0f,
        0.0f,
        static_cast<float>(GetWindowWidth()),
        static_cast<float>(GetWindowHeight()),
        0.0f,
        1.0f
    };

    _deviceContext->RSSetViewports(1, &viewport);
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    _shaderCollection.ApplyToContext(_deviceContext.Get());


    _deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

    _deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
    _deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

    UINT stride = _shaderCollection.GetLayoutByteSize(VertexType::PositionColor);
    
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices[0].GetAddressOf(),
        &stride,
        &vertexOffset);

    _deviceContext->RSSetState(_solidFrameCullNoneRasterizerState.Get());
    _deviceContext->Draw(3, 0);

    _deviceContext->RSSetState(_solidFrameCullFrontRasterizerState.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices[1].GetAddressOf(),
        &stride,
        &vertexOffset);
    _deviceContext->Draw(3, 0);

    _deviceContext->RSSetState(_solidFrameCullBackRasterizerState.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices[2].GetAddressOf(),
        &stride,
        &vertexOffset);
    _deviceContext->Draw(3, 0);

    _deviceContext->RSSetState(_wireFrameCullNoneRasterizerState.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices[3].GetAddressOf(),
        &stride,
        &vertexOffset);
    _deviceContext->Draw(3, 0);

    _deviceContext->RSSetState(_wireFrameCullFrontRasterizerState.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices[4].GetAddressOf(),
        &stride,
        &vertexOffset);
    _deviceContext->Draw(3, 0);

    _deviceContext->RSSetState(_wireFrameCullBackRasterizerState.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices[5].GetAddressOf(),
        &stride,
        &vertexOffset);
    _deviceContext->Draw(3, 0);

    _swapChain->Present(1, 0);
}

bool RasterizerStateApplication::CreateRasterizerStates()
{
    D3D11_RASTERIZER_DESC rasterizerStateDescriptor = {};
    rasterizerStateDescriptor.AntialiasedLineEnable = false;
    rasterizerStateDescriptor.DepthBias = 0;
    rasterizerStateDescriptor.DepthBiasClamp = 0.0f;
    rasterizerStateDescriptor.DepthClipEnable = false;
    rasterizerStateDescriptor.FrontCounterClockwise = false;
    rasterizerStateDescriptor.MultisampleEnable = false;
    rasterizerStateDescriptor.ScissorEnable = false;
    rasterizerStateDescriptor.SlopeScaledDepthBias = 0.0f;

    rasterizerStateDescriptor.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;

    rasterizerStateDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    if (FAILED(_device->CreateRasterizerState(&rasterizerStateDescriptor, &_solidFrameCullBackRasterizerState)))
    {
        std::cout << "D3D11: Failed to create rasterizer state\n";
        return false;
    }

    rasterizerStateDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
    if (FAILED(_device->CreateRasterizerState(&rasterizerStateDescriptor, &_solidFrameCullFrontRasterizerState)))
    {
        std::cout << "D3D11: Failed to create rasterizer state\n";
        return false;
    }

    rasterizerStateDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    if (FAILED(_device->CreateRasterizerState(&rasterizerStateDescriptor, &_solidFrameCullNoneRasterizerState)))
    {
        std::cout << "D3D11: Failed to create rasterizer state\n";
        return false;
    }

    rasterizerStateDescriptor.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

    rasterizerStateDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    if (FAILED(_device->CreateRasterizerState(&rasterizerStateDescriptor, &_wireFrameCullBackRasterizerState)))
    {
        std::cout << "D3D11: Failed to create rasterizer state\n";
        return false;
    }

    rasterizerStateDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
    if (FAILED(_device->CreateRasterizerState(&rasterizerStateDescriptor, &_wireFrameCullFrontRasterizerState)))
    {
        std::cout << "D3D11: Failed to create rasterizer state\n";
        return false;
    }

    rasterizerStateDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    if (FAILED(_device->CreateRasterizerState(&rasterizerStateDescriptor, &_wireFrameCullNoneRasterizerState)))
    {
        std::cout << "D3D11: Failed to create rasterizer state\n";
        return false;
    }

    return true;
}
