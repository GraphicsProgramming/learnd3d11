#include "HelloTriangle.hpp"

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

HelloTriangleApplication::HelloTriangleApplication(const std::string_view title)
    : Application(title)
{
}

HelloTriangleApplication::~HelloTriangleApplication()
{
    _deviceContext->Flush();
    _pixelShader.Reset();
    _vertexShader.Reset();
    _vertexLayout.Reset();
    _triangleVertices.Reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.Reset();
    _device.Reset();
    Application::Cleanup();
}

bool HelloTriangleApplication::Initialize()
{
    // This section initializes GLFW and creates a Window
    if (!Application::Initialize())
    {
        return false;
    }

    // This section initializes DirectX's devices and SwapChain
    if (FAILED(CreateDXGIFactory1(
        __uuidof(IDXGIFactory1),
        &_dxgiFactory)))
    {
        std::cout << "DXGI: Unable to create DXGIFactory\n";
        return false;
    }

    constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
    UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;
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
    swapchainInfo.OutputWindow = glfwGetWin32Window(GetWindow());
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

    CreateSwapchainResources();
    // This section reads and compiles both the vertex and pixel shaders
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3D10Blob> vertexShaderBlob = nullptr;
    ComPtr<ID3D10Blob> pixelShaderBlob = nullptr;
    ComPtr<ID3D10Blob> errorBlob = nullptr;
    if (FAILED(D3DCompileFromFile(
        L"Assets/Shaders/Main.vs.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        compileFlags,
        0,
        &vertexShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Failed to read vertex shader from file\n";
        if (errorBlob)
        {
            std::cout << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        }
        return false;
    }
    if (FAILED(D3DCompileFromFile(
        L"Assets/Shaders/Main.ps.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_0",
        compileFlags,
        0,
        &pixelShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Failed to read pixel shader from file\n";
        if (errorBlob)
        {
            std::cout << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        }
        return false;
    }

    if (FAILED(_device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &_vertexShader)))
    {
        std::cout << "D3D11: Failed to compile vertex shader\n";
        return false;
    }
    if (FAILED(_device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &_pixelShader)))
    {
        std::cout << "D3D11: Failed to compile pixel shader\n";
        return false;
    }
    constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexPositionColor, position),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0
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
    };
    if (FAILED(_device->CreateInputLayout(
        vertexInputLayoutInfo,
        _countof(vertexInputLayoutInfo),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &_vertexLayout)))
    {
        std::cout << "D3D11: Failed to create default vertex input layout\n";
    }
    // This section to create the vertex and index buffer of the triangle
    const VertexPositionColor vertices[] =
    {
        { Position{  0.0f,  0.5f, 0.0f }, Color{ 0.25f, 0.39f, 0.19f } },
        { Position{  0.5f, -0.5f, 0.0f }, Color{ 0.44f, 0.75f, 0.35f } },
        { Position{ -0.5f, -0.5f, 0.0f }, Color{ 0.38f, 0.55f, 0.20f } },
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

bool HelloTriangleApplication::CreateSwapchainResources()
{
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

void HelloTriangleApplication::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void HelloTriangleApplication::OnResize(
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

void HelloTriangleApplication::Update()
{
}

void HelloTriangleApplication::Render()
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = GetWindowWidth();
    viewport.Height = GetWindowHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    constexpr UINT vertexStride = sizeof(VertexPositionColor);
    constexpr UINT vertexOffset = 0;

    _deviceContext->ClearRenderTargetView(
        _renderTarget.Get(),
        clearColor);
    _deviceContext->IASetInputLayout(_vertexLayout.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices.GetAddressOf(),
        &vertexStride,
        &vertexOffset);
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _deviceContext->VSSetShader(
        _vertexShader.Get(),
        nullptr,
        0);
    _deviceContext->RSSetViewports(
        1,
        &viewport);
    _deviceContext->PSSetShader(
        _pixelShader.Get(),
        nullptr,
        0);
    _deviceContext->OMSetRenderTargets(
        1,
        _renderTarget.GetAddressOf(),
        nullptr);
    _deviceContext->Draw(3, 0);
    _swapChain->Present(1, 0);
}