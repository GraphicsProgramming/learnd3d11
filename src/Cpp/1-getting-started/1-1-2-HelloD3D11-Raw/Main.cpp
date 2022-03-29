#include <d3d11_2.h>
#include <wrl.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string_view>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

int32_t g_Width = 0;
int32_t g_Height = 0;

ComPtr<ID3D11Device> g_Device = nullptr;
ComPtr<ID3D11DeviceContext> g_DeviceContext = nullptr;
ComPtr<IDXGIFactory2> g_DxgiFactory = nullptr;
ComPtr<IDXGISwapChain1> g_SwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> g_RenderTarget = nullptr;

bool CreateSwapchainResources()
{
    ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    if (FAILED(g_SwapChain->GetBuffer(
        0,
        IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get Back Buffer from the SwapChain\n";
        return false;
    }

    if (FAILED(g_Device->CreateRenderTargetView(
        backBuffer.Get(),
        nullptr,
        &g_RenderTarget)))
    {
        std::cout << "D3D11: Failed to create RTV from Back Buffer\n";
        return false;
    }

    return true;
}

void DestroySwapchainResources()
{
    g_RenderTarget.Reset();
}

void HandleResize(
    GLFWwindow* window,
    const int32_t width,
    const int32_t height)
{
    g_Width = width;
    g_Height = height;

    g_DeviceContext->Flush();

    DestroySwapchainResources();
    if (FAILED(g_SwapChain->ResizeBuffers(
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

int main()
{
    if (!glfwInit())
    {
        std::cout << "Unable to initialize GLFW\n";
        return false;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    g_Width = static_cast<int32_t>(videoMode->width * 0.9f);
    g_Height = static_cast<int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        g_Width,
        g_Height,
        "LearnD3D11 - HelloD3D11",
        nullptr,
        nullptr);

    if (window == nullptr)
    {
        std::cout << "GLFW: Unable to create window\n";
        glfwTerminate();
        return false;
    }

    const int32_t windowLeft = videoMode->width / 2 - g_Width / 2;
    const int32_t windowTop = videoMode->height / 2 - g_Height / 2;
    glfwSetWindowPos(window, windowLeft, windowTop);

    glfwSetFramebufferSizeCallback(window, HandleResize);

    // This section initializes DirectX's devices and SwapChain
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&g_DxgiFactory))))
    {
        std::cout << "DXGI: Unable to create DXGIFactory\n";
        return false;
    }

    constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

    if (FAILED(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        &deviceFeatureLevel,
        1,
        D3D11_SDK_VERSION,
        &g_Device,
        nullptr,
        &g_DeviceContext)))
    {
        std::cout << "D3D11: Failed to create Device and Device Context\n";
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
    swapChainDescriptor.Width = g_Width;
    swapChainDescriptor.Height = g_Height;
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

    if (FAILED(g_DxgiFactory->CreateSwapChainForHwnd(
        g_Device.Get(),
        glfwGetWin32Window(window),
        &swapChainDescriptor,
        &swapChainFullscreenDescriptor,
        nullptr,
        &g_SwapChain)))
    {
        std::cout << "DXGI: Failed to create SwapChain\n";
        return false;
    }

    if (!CreateSwapchainResources())
    {
        return false;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = g_Width;
        viewport.Height = g_Height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

        g_DeviceContext->ClearRenderTargetView(
            g_RenderTarget.Get(),
            clearColor);
        g_DeviceContext->RSSetViewports(
            1,
            &viewport);
        g_DeviceContext->OMSetRenderTargets(
            1,
            g_RenderTarget.GetAddressOf(),
            nullptr);
        g_SwapChain->Present(1, 0);
    }

    return 0;
}
