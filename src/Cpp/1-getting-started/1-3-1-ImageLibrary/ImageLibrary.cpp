#include "ImageLibrary.hpp"
#include "DeviceContext.hpp"
#include "Pipeline.hpp"
#include "PipelineFactory.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <DirectXTex.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

template<UINT TDebugNameLength>
inline void SetDebugName(_In_ ID3D11DeviceChild* deviceResource, _In_z_ const char(&debugName)[TDebugNameLength])
{
    deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
}

ImageLibraryApplication::ImageLibraryApplication(const std::string_view title)
    : Application(title)
{
}

ImageLibraryApplication::~ImageLibraryApplication()
{
    _deviceContext->Flush();
    _textureSrv.Reset();
    _triangleVertices.Reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
    Application::Cleanup();
}

bool ImageLibraryApplication::Initialize()
{
    // This section initializes GLFW and creates a Window
    if (!Application::Initialize())
    {
        return false;
    }

    // This section initializes DirectX's devices and SwapChain
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
    {
        std::cout << "DXGI: Unable to create DXGIFactory\n";
        return false;
    }

    constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
    UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;
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

    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }

    constexpr char deviceName[] = "DEV_Main";
    _device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(deviceContext.Get(), "CTX_Main");

    _deviceContext = std::make_unique<DeviceContext>(std::move(deviceContext));

    DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
    swapChainDescriptor.Width = GetWindowWidth();
    swapChainDescriptor.Height = GetWindowHeight();
    swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDescriptor.SampleDesc.Count = 1;
    swapChainDescriptor.SampleDesc.Quality = 0;
    swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescriptor.BufferCount = 2;
    swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
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
    PipelineSettings pipelineSettings = {};
    pipelineSettings.VertexFilePath = L"Assets/Shaders/Main.vs.hlsl";
    pipelineSettings.PixelFilePath = L"Assets/Shaders/Main.ps.hlsl";
    pipelineSettings.VertexType = VertexType::PositionColorUv;
    _pipelineFactory->CreatePipeline(pipelineSettings, _pipeline);

    constexpr VertexPositionColorUv vertices[] =
    {
        { Position{  0.0f,  0.5f, 0.0f }, Color{ 0.25f, 0.39f, 0.19f }, Uv{ 0.5f, 0.0f } },
        { Position{  0.5f, -0.5f, 0.0f }, Color{ 0.44f, 0.75f, 0.35f }, Uv{ 1.0f, 1.0f } },
        { Position{ -0.5f, -0.5f, 0.0f }, Color{ 0.38f, 0.55f, 0.20f }, Uv{ 0.0f, 1.0f } },
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

    DirectX::TexMetadata metaData = {};
    DirectX::ScratchImage scratchImage;
    if (FAILED(DirectX::LoadFromDDSFile(L"Assets/Textures/T_Froge.dds", DirectX::DDS_FLAGS_NONE, &metaData, scratchImage)))
    {
        std::cout << "DXTEX: Unable to load image\n";
        return false;
    }

    WRL::ComPtr<ID3D11Resource> texture = nullptr;
    if (FAILED(DirectX::CreateTexture(
        _device.Get(),
        scratchImage.GetImages(),
        scratchImage.GetImageCount(),
        metaData,
        &texture)))
    {
        std::cout << "DXTEX: Unable to create texture out of image\n";
        scratchImage.Release();
        return false;
    }

    if (FAILED(DirectX::CreateShaderResourceView(
        _device.Get(),
        scratchImage.GetImages(),
        scratchImage.GetImageCount(),
        metaData,
        &_textureSrv)))
    {
        std::cout << "DXTEX: Unable to create shader resource view out of texture\n";
        scratchImage.Release();
        return false;
    }

    _pipeline->BindTexture(0, _textureSrv.Get());

    D3D11_SAMPLER_DESC linearSamplerStateDescriptor = {};
    linearSamplerStateDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    linearSamplerStateDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    linearSamplerStateDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    linearSamplerStateDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    if (FAILED(_device->CreateSamplerState(&linearSamplerStateDescriptor, &_linearSamplerState)))
    {
        std::cout << "D3D11: Unable to create linear sampler state\n";
        return false;
    }

    _pipeline->BindSampler(0, _linearSamplerState.Get());
    
    return true;
}

bool ImageLibraryApplication::CreateSwapchainResources()
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

void ImageLibraryApplication::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void ImageLibraryApplication::OnResize(
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

void ImageLibraryApplication::Update()
{
}

void ImageLibraryApplication::Render()
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = GetWindowWidth();
    viewport.Height = GetWindowHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    constexpr float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    constexpr uint32_t vertexStride = sizeof(VertexPositionColorUv);
    constexpr uint32_t vertexOffset = 0;

    _deviceContext->Clear(_renderTarget.Get(), clearColor);
    _deviceContext->SetPipeline(_pipeline.get());
    _deviceContext->SetVertexBuffer(_triangleVertices.Get(), vertexOffset);
    _deviceContext->SetViewport(viewport);
    _deviceContext->Draw();
    _swapChain->Present(1, 0);
}
