#include "HelloD3D11.hpp"

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

struct VertexFormat
{
    float vertex[3];
    float color[3];
};

HelloD3D11Application::HelloD3D11Application(const std::string_view title)
    : Application(title)
{
}

HelloD3D11Application::~HelloD3D11Application()
{
    _deviceContext->Flush();
    _pixelShader->Release();
    _vertexShader->Release();
    _vertexLayout->Release();
    _triangleIndices->Release();
    _triangleVerts->Release();
    _renderTarget->Release();
    _swapChain->Release();
    _dxgiFactory->Release();
    _deviceContext->Release();
    _debugInfoQueue->Release();
    _debug->Release();
    _device->Release();
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
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&_dxgiFactory))))
    {
        std::cout << "DXGI: Unable to create DXGIFactory\n";
        return false;
    }
    _dxgiFactory->MakeWindowAssociation(nativeWindow, 0);
    constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    if (FAILED(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
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
    if (FAILED(_device->QueryInterface(&_debug)))
    {
        std::cout << "D3D11: Failed to get Device Debug Context\n";
        return false;
    }
    if (FAILED(_debug->QueryInterface(&_debugInfoQueue)))
    {
        std::cout << "D3D11: Failed to get Debug Info Queue\n";
        return false;
    }
    _debugInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
    _debugInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

    D3D11_MESSAGE_ID hide[] =
    {
        D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
        // Add more message IDs here as needed
    };

    D3D11_INFO_QUEUE_FILTER filter = {};
    filter.DenyList.NumIDs = _countof(hide);
    filter.DenyList.pIDList = hide;
    _debugInfoQueue->AddStorageFilterEntries(&filter);
    _debugInfoQueue->Release();
#endif

    DXGI_SWAP_CHAIN_DESC swapchainInfo;
    swapchainInfo.BufferDesc.Width = GetWindowWidth();
    swapchainInfo.BufferDesc.Height = GetWindowHeight();
    swapchainInfo.BufferDesc.RefreshRate.Numerator = 0;
    swapchainInfo.BufferDesc.RefreshRate.Denominator = 1;
    swapchainInfo.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapchainInfo.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapchainInfo.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    swapchainInfo.SampleDesc.Count = 1;
    swapchainInfo.SampleDesc.Quality = 0;
    swapchainInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainInfo.BufferCount = 2;
    swapchainInfo.OutputWindow = nativeWindow;
    swapchainInfo.Windowed = true;
    swapchainInfo.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchainInfo.Flags = {};

    if (FAILED(_dxgiFactory->CreateSwapChain(_device, &swapchainInfo, &_swapChain)))
    {
        std::cout << "DXGI: Failed to create SwapChain\n";
        return false;
    }

    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get Back Buffer from the SwapChain\n";
        return false;
    }

    if (FAILED(_device->CreateRenderTargetView(backBuffer, nullptr, &_renderTarget)))
    {
        std::cout << "D3D11: Failed to create RTV from Back Buffer\n";
        backBuffer->Release();
        return false;
    }
    backBuffer->Release();
    // This section reads and compiles both the vertex and pixel shaders
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(NDEBUG)
    compileFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3D10Blob* vertexShaderBlob;
    ID3D10Blob* pixelShaderBlob;
    ID3D10Blob* errorBlob;
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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexFormat, vertex), D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexFormat, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
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
    const VertexFormat vertices[] =
    {
        //         Vertex                  Color
        { {  0.0f,  0.5f, 0.0f }, { 0.25f, 0.39f, 0.19f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.44f, 0.75f, 0.35f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.38f, 0.55f, 0.20f } },
    };
    D3D11_BUFFER_DESC bufferInfo = {};
    bufferInfo.ByteWidth = sizeof vertices;
    bufferInfo.Usage = D3D11_USAGE_IMMUTABLE;
    bufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA resourceData = {};
    resourceData.pSysMem = vertices;
    if (FAILED(_device->CreateBuffer(
        &bufferInfo,
        &resourceData,
        &_triangleVerts)))
    {
        std::cout << "D3D11: Failed to create triangle vertex buffer\n";
        return false;
    }
    const uint32_t indices[] =
    {
        0, 1, 2
    };
    bufferInfo.ByteWidth = sizeof indices;
    bufferInfo.BindFlags = D3D11_BIND_INDEX_BUFFER;
    resourceData.pSysMem = indices;
    if (FAILED(_device->CreateBuffer(
        &bufferInfo,
        &resourceData,
        &_triangleIndices)))
    {
        std::cout << "D3D11: Failed to create index buffer\n";
        return false;
    }
    return true;
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
    const UINT vertexStride = sizeof(VertexFormat);
    const UINT vertexOffset = 0;

    _deviceContext->OMSetRenderTargets(1, &_renderTarget, nullptr);
    _deviceContext->RSSetViewports(1, &viewport);
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _deviceContext->IASetInputLayout(_vertexLayout);
    _deviceContext->IASetIndexBuffer(_triangleIndices, DXGI_FORMAT_R32_UINT, 0);
    _deviceContext->IASetVertexBuffers(0, 1, &_triangleVerts, &vertexStride, &vertexOffset);
    _deviceContext->VSSetShader(_vertexShader, nullptr, 0);
    _deviceContext->PSSetShader(_pixelShader, nullptr, 0);
    _deviceContext->ClearRenderTargetView(_renderTarget, clearColor);
    _deviceContext->DrawIndexed(3, 0, 0);
    _swapChain->Present(0, 0);
}

