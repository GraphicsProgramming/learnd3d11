#include "HelloD3D11.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;

struct VertexPositionColor
{
    Position position;
    Color color;
};

HelloD3D11Application::HelloD3D11Application(const std::string_view title)
    : Application(title)
{
}

HelloD3D11Application::~HelloD3D11Application()
{
    _deviceContext->Flush();
    _renderTarget.Reset();
    _swapChain.Reset();
    _dxgiFactory.Reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
    _debug.Reset();
#endif
    _deviceContext.Reset();
    _device.Reset();
    Application::Cleanup();
}

bool HelloD3D11Application::Initialize()
{
    // This section initializes GLFW and creates a Window
    if (!Application::Initialize())
    {
        return false;
    }
    const HWND nativeWindow = glfwGetWin32Window(GetWindow());
    // This section initializes DirectX's devices and SwapChain
    if (FAILED(CreateDXGIFactory1(
        __uuidof(IDXGIFactory1),
        &_dxgiFactory)))
    {
        std::cout << "DXGI: Unable to create DXGIFactory\n";
        return false;
    }
    _dxgiFactory->MakeWindowAssociation(nativeWindow, 0);
    constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
    UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif
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
        &_deviceContext)))
    {
        std::cout << "D3D11: Failed to create Device and Device Context\n";
        return false;
    }
#if !defined(NDEBUG)
    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get Device Debug Context\n";
        return false;
    }
    if (FAILED(_debug.As(&_debugInfoQueue)))
    {
        std::cout << "D3D11: Failed to get Debug Info Queue\n";
        return false;
    }
    _debugInfoQueue->SetBreakOnSeverity(
        D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_CORRUPTION,
        true);
    _debugInfoQueue->SetBreakOnSeverity(
        D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_ERROR,
        true);

    D3D11_MESSAGE_ID hide[] =
    {
        D3D11_MESSAGE_ID::D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
        // Add more message IDs here as needed
    };

    D3D11_INFO_QUEUE_FILTER filter = {};
    filter.DenyList.NumIDs = _countof(hide);
    filter.DenyList.pIDList = hide;
    _debugInfoQueue->AddStorageFilterEntries(&filter);
    _debugInfoQueue.Reset();
#endif

    DXGI_SWAP_CHAIN_DESC swapchainInfo = {};
    swapchainInfo.BufferDesc.Width = GetWindowWidth();
    swapchainInfo.BufferDesc.Height = GetWindowHeight();
    swapchainInfo.BufferDesc.RefreshRate.Numerator = 0;
    swapchainInfo.BufferDesc.RefreshRate.Denominator = 1;
    swapchainInfo.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchainInfo.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapchainInfo.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_STRETCHED;
    swapchainInfo.SampleDesc.Count = 1;
    swapchainInfo.SampleDesc.Quality = 0;
    swapchainInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainInfo.BufferCount = 2;
    swapchainInfo.OutputWindow = nativeWindow;
    swapchainInfo.Windowed = true;
    swapchainInfo.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainInfo.Flags = {};
    
    if (FAILED(_dxgiFactory->CreateSwapChain(
        _device.Get(),
        &swapchainInfo,
        &_swapChain)))
    {
        std::cout << "DXGI: Failed to create SwapChain\n";
        return false;
    }

    ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    if (FAILED(_swapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        &backBuffer)))
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

void HelloD3D11Application::OnResize(int32_t width, int32_t height)
{
    Application::OnResize(width, height);
    _deviceContext->Flush();
    _renderTarget.Reset();
    if (FAILED(_swapChain->ResizeBuffers(
        0,
        width,
        height,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        0)))
    {
        std::cout << "D3D11: Failed to recreate SwapChain buffers\n";
        return;
    }
    ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    if (FAILED(_swapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        &backBuffer)))
    {
        std::cout << "D3D11: Failed to acquire back buffer from the SwapChain\n";
        return;
    }
    if (FAILED(_device->CreateRenderTargetView(
        backBuffer.Get(),
        nullptr,
        &_renderTarget)))
    {
        std::cout << "D3D11: Failed to create Render Target from the back buffer\n";
    }
}

void HelloD3D11Application::Update()
{
}

void HelloD3D11Application::Render()
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = GetWindowWidth();
    viewport.Height = GetWindowHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    const UINT vertexStride = sizeof(VertexPositionColor);
    const UINT vertexOffset = 0;

    _deviceContext->ClearRenderTargetView(
        _renderTarget.Get(),
        clearColor);
    _deviceContext->RSSetViewports(
        1,
        &viewport);
    _deviceContext->OMSetRenderTargets(
        1,
        _renderTarget.GetAddressOf(),
        nullptr);
    _swapChain->Present(0, 0);
}

