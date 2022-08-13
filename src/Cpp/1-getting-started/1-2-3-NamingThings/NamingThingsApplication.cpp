#include "NamingThingsApplication.hpp"
#include "DeviceContext.hpp"
#include "Pipeline.hpp"
#include "PipelineFactory.hpp"
#include "VertexType.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXColors.h>
#include <DirectXMath.h>
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

NamingThingsApplication::NamingThingsApplication(const std::string& title)
    : Application(title)
{
}

NamingThingsApplication::~NamingThingsApplication()
{
    _deviceContext->Flush();
    _triangleVertices.Reset();
    _pipeline.reset();
    _pipelineFactory.reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
}

bool NamingThingsApplication::Initialize()
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

    WRL::ComPtr<ID3D11DeviceContext> deviceContext;
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
        std::cout << "D3D11: Failed to create Device and Device Context\n";
        return false;
    }

    constexpr char deviceName[] = "DEV_Main";
    _device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(deviceContext.Get(), "CTX_Main");

    _deviceContext = std::make_unique<DeviceContext>(std::move(deviceContext));

    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }

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
        std::cout << "DXGI: Failed to create SwapChain\n";
        return false;
    }

    CreateSwapchainResources();

    _pipelineFactory = std::make_unique<PipelineFactory>(_device);

    return true;
}

bool NamingThingsApplication::Load()
{
    PipelineDescriptor pipelineDescriptor = {};
    pipelineDescriptor.VertexFilePath = L"Assets/Shaders/Main.vs.hlsl";
    pipelineDescriptor.PixelFilePath = L"Assets/Shaders/Main.ps.hlsl";
    pipelineDescriptor.VertexType = VertexType::PositionColor;
    if (!_pipelineFactory->CreatePipeline(pipelineDescriptor, _pipeline))
    {
        std::cout << "PipelineFactory: Failed to create pipeline\n";
        return false;
    }

    _pipeline->SetViewport(
        0.0f,
        0.0f,
        static_cast<float>(GetWindowWidth()),
        static_cast<float>(GetWindowHeight()));

    constexpr VertexPositionColor vertices[] = {
        {  Position{ 0.0f, 0.5f, 0.0f }, Color{ 0.25f, 0.39f, 0.19f }},
        { Position{ 0.5f, -0.5f, 0.0f }, Color{ 0.44f, 0.75f, 0.35f }},
        {Position{ -0.5f, -0.5f, 0.0f }, Color{ 0.38f, 0.55f, 0.20f }},
    };
    D3D11_BUFFER_DESC bufferInfo = {};
    bufferInfo.ByteWidth = sizeof(vertices);
    bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA resourceData = {};
    resourceData.pSysMem = vertices;

    if (FAILED(_device->CreateBuffer(
            &bufferInfo,
            &resourceData,
            &_triangleVertices)))
    {
        std::cout << "D3D11: Failed to create triangle vertex buffer\n";
        return false;
    }

    return true;
}

bool NamingThingsApplication::CreateSwapchainResources()
{
    WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    if (FAILED(_swapChain->GetBuffer(
            0,
            IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get Back Buffer from the SwapChain\n";
        return false;
    }

    if (FAILED(_device->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            &_renderTarget)))
    {
        std::cout << "D3D11: Failed to create RTV from Back Buffer\n";
        return false;
    }

    return true;
}

void NamingThingsApplication::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void NamingThingsApplication::OnResize(
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
        std::cout << "D3D11: Failed to recreate SwapChain buffers\n";
        return;
    }

    CreateSwapchainResources();
}

void NamingThingsApplication::Update()
{
    Application::Update();
}

void NamingThingsApplication::Render()
{
    float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    constexpr uint32_t vertexOffset = 0;

    _deviceContext->Clear(_renderTarget.Get(), clearColor);
    _deviceContext->SetPipeline(_pipeline.get());
    _deviceContext->SetVertexBuffer(_triangleVertices.Get(), vertexOffset);
    _deviceContext->Draw();
    _swapChain->Present(1, 0);
}
