#include "D3DContext.hpp"

#include <iostream>
#include <cassert>
#include <utility>

#include "../Application.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;

namespace std
{
    template<> struct hash<GUID>
    {
        size_t operator()(const GUID& guid) const noexcept
        {
            const auto* p = reinterpret_cast<const std::uint64_t*>(&guid);
            constexpr std::hash<std::uint64_t> hash;
            return hash(p[0]) ^ hash(p[1]);
        }
    };
}

bool D3DContext::Initialize(const Application& application)
{
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(_dxgiFactory.GetAddressOf()))))
    {
        std::cout << "DXGI: Unable to create DXGIFactory\n";
        return false;
    }
    _dxgiFactory->MakeWindowAssociation(application.GetWindowHandle(), 0);
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
        _device.ReleaseAndGetAddressOf(),
        nullptr,
        _deviceContext.ReleaseAndGetAddressOf())))
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
    swapchainInfo.BufferDesc.Width = application.GetWindowWidth();
    swapchainInfo.BufferDesc.Height = application.GetWindowHeight();
    swapchainInfo.BufferDesc.RefreshRate.Numerator = 0;
    swapchainInfo.BufferDesc.RefreshRate.Denominator = 1;
    swapchainInfo.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapchainInfo.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapchainInfo.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    swapchainInfo.SampleDesc.Count = 1;
    swapchainInfo.SampleDesc.Quality = 0;
    swapchainInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainInfo.BufferCount = 2;
    swapchainInfo.OutputWindow = application.GetWindowHandle();
    swapchainInfo.Windowed = true;
    swapchainInfo.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchainInfo.Flags = {};

    if (FAILED(_dxgiFactory->CreateSwapChain(_device.Get(), &swapchainInfo, &_swapChain)))
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
    return true;
}

bool D3DContext::MakeGraphicsPipeline(
    const wchar_t* vertexPath,
    const wchar_t* pixelPath,
    GraphicsPipeline& outPipeline) const
{
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(NDEBUG)
    compileFlags |= D3DCOMPILE_DEBUG;
#endif
    ComPtr<ID3D10Blob> vertexShaderBlob;
    ComPtr<ID3D10Blob> pixelShaderBlob;
    ComPtr<ID3D10Blob> errorBlob;
    if (FAILED(D3DCompileFromFile(
        vertexPath,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        compileFlags,
        0,
        &vertexShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Unable to create vertex shader from file: " << vertexPath << "\n";
        if (errorBlob)
        {
            std::cout << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        }
        return false;
    }
    if (FAILED(D3DCompileFromFile(
        pixelPath,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_0",
        compileFlags,
        0,
        &pixelShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Unable to create pixel shader from file: " << pixelPath << "\n";
        if (errorBlob)
        {
            std::cout << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        }
        return false;
    }

    ComPtr<ID3D11VertexShader> vertexShader = nullptr;
    if (FAILED(_device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        std::cout << "D3D11: Failed to create vertex shader from file: " << vertexPath << "\n";
        return false;
    }
    ComPtr<ID3D11PixelShader> pixelShader;
    if (FAILED(_device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader)))
    {
        std::cout << "D3D11: Failed to create pixel shader from file: " << pixelPath << "\n";
        return false;
    }
    ComPtr<ID3D11InputLayout> inputLayout;
    constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (FAILED(_device->CreateInputLayout(
        vertexInputLayoutInfo,
        _countof(vertexInputLayoutInfo),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &inputLayout)))
    {
        std::cout << "D3D11: Failed to create default vertex input layout\n";
    }
    outPipeline.vertexShader = std::move(vertexShader);
    outPipeline.pixelShader = std::move(pixelShader);
    outPipeline.inpuLayout = std::move(inputLayout);
    return true;
}
