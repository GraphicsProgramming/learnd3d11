#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// System includes
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wincodec.h>
#include <fstream>

#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "../Shared/CommonLibraries.h"

template<UINT TDebugNameLength>
inline void SetDebugName(_In_ ID3D11DeviceChild* deviceResource, _In_z_ const char(&debugName)[TDebugNameLength])
{
    deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
}

struct VertexPositionColorUv
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Color;
    DirectX::XMFLOAT2 Uv;
};

enum ConstantBufferType
{
    PerApplication,
    PerFrame,
    PerObject,
    NumConstantBufferTypes
};

ComPtr<IDXGIFactory1> g_Factory;

GLFWwindow* g_Window = nullptr;

ComPtr<ID3D11Debug> g_Debug = nullptr;
ComPtr<ID3D11Device> g_Device = nullptr;
ComPtr<ID3D11DeviceContext> g_DeviceContext = nullptr;
DXGI_SWAP_CHAIN_DESC g_SwapChainDescriptor = {};
ComPtr<IDXGISwapChain> g_SwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> g_BackBufferRtv = nullptr;
ComPtr<ID3D11DepthStencilView> g_DepthStencilView = nullptr;
ComPtr<ID3D11Texture2D> g_DepthStencilBuffer = nullptr;
ComPtr<ID3D11DepthStencilState> g_DepthStencilState = nullptr;
ComPtr<ID3D11RasterizerState> g_RasterizerState = nullptr;
D3D11_VIEWPORT g_ViewPort = {};

ComPtr<ID3D11InputLayout> g_InputLayout = nullptr;
ComPtr<ID3D11VertexShader> g_VertexShader = nullptr;
ComPtr<ID3DBlob> g_VertexShaderBlob = nullptr;
ComPtr<ID3D11PixelShader> g_PixelShader = nullptr;

IWICImagingFactory* g_ImagingFactory = nullptr;
ComPtr<ID3D11ShaderResourceView> g_TextureSrv = nullptr;
ComPtr<ID3D11SamplerState> g_LinearSamplerState = nullptr;

ComPtr<ID3D11Buffer> g_VertexBuffer = nullptr;
ComPtr<ID3D11Buffer> g_IndexBuffer = nullptr;
ComPtr<ID3D11Buffer> g_ConstantBuffers[NumConstantBufferTypes];

constexpr float g_ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

constexpr std::uint32_t g_VertexBufferStride = sizeof(VertexPositionColorUv);
VertexPositionColorUv g_Vertices[8] =
{
    { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
    { DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }
};

std::uint16_t g_Indices[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

DirectX::XMMATRIX g_WorldMatrix;
DirectX::XMMATRIX g_ViewMatrix;
DirectX::XMMATRIX g_ProjectionMatrix;

void CenterWindow(GLFWwindow* window)
{
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    const auto primaryMonitor = glfwGetPrimaryMonitor();
    const auto videoMode = glfwGetVideoMode(primaryMonitor);
    const auto windowLeft = videoMode->width / 2 - windowWidth / 2;
    const auto windowTop = videoMode->height / 2 - windowHeight / 2;
    glfwSetWindowPos(window, windowLeft, windowTop);
}

void OnWindowResize(GLFWwindow* window, const int width, const int height)
{
    /*
    if (g_SwapChain == nullptr)
    {
        return;
    }

    g_BackBufferRtv->Release();
    if (FAILED(g_SwapChain->ResizeBuffers(2, width, height, g_SwapChainDescriptor.BufferDesc.Format, g_SwapChainDescriptor.Flags)))
    {
        std::cout << "D3D11: Unable to resize swapchain\n";
        return;
    }
    */
}

bool Initialize()
{
    if (!glfwInit())
    {
        std::cout << "GLFW: Unable to initialize\n";
        return false;
    }

    constexpr int windowWidth = 1920;
    constexpr int windowHeight = 1080;
    g_Window = glfwCreateWindow(windowWidth, windowHeight, "LearnD3D11 - Hello Froge", nullptr, nullptr);
    if (g_Window == nullptr)
    {
        std::cout << "GLFW: Unable to create a window\n";
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(g_Window, OnWindowResize);
    CenterWindow(g_Window);

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&g_Factory))))
    {
        std::cout << "DXGI: Unable to create factory\n";
        return false;
    }

    constexpr char factoryName[] = "Factory1";
    g_Factory->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(factoryName), factoryName);

    constexpr D3D_FEATURE_LEVEL d3dFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0
    };
    constexpr UINT deviceCreationFlags =
        D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT |
        D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
    if (FAILED(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceCreationFlags,
        d3dFeatureLevels,
        1,
        D3D11_SDK_VERSION,
        g_Device.ReleaseAndGetAddressOf(),
        nullptr,
        g_DeviceContext.ReleaseAndGetAddressOf())))
    {
        std::cout << "D3D11: Device creation failed\n";
        return false;
    }

    constexpr char deviceName[] = "DEV_Main";
    g_Device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(g_DeviceContext.Get(), "CTX_Main");

    if (FAILED(g_Device.As(&g_Debug)))
    {
        std::cout << "D3D11: Unable to grab the debug interface from device\n";
        return false;
    }

    ComPtr<ID3D11InfoQueue> debugInfoQueue = nullptr;
    if (FAILED(g_Debug.As(&debugInfoQueue)))
    {
        std::cout << "D3D11: Unable to grab the info queue interface from the debug interface\n";
        return false;
    }

    debugInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
    debugInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_ERROR, true);

    D3D11_MESSAGE_ID hide[] =
    {
        D3D11_MESSAGE_ID::D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
        // Add more message IDs here as needed
    };

    D3D11_INFO_QUEUE_FILTER filter = {};
    filter.DenyList.NumIDs = _countof(hide);
    filter.DenyList.pIDList = hide;
    debugInfoQueue->AddStorageFilterEntries(&filter);
    debugInfoQueue->Release();


    const auto nativeWindowHandle = glfwGetWin32Window(g_Window);
    g_Factory->MakeWindowAssociation(nativeWindowHandle, 0);

    constexpr DXGI_RATIONAL refreshRate = { 0, 1 };
    g_SwapChainDescriptor.BufferCount = 2;
    g_SwapChainDescriptor.BufferDesc.Width = windowWidth;
    g_SwapChainDescriptor.BufferDesc.Height = windowHeight;
    g_SwapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    g_SwapChainDescriptor.BufferDesc.RefreshRate = refreshRate;
    g_SwapChainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    g_SwapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    g_SwapChainDescriptor.OutputWindow = nativeWindowHandle;
    g_SwapChainDescriptor.SampleDesc.Count = 1;
    g_SwapChainDescriptor.SampleDesc.Quality = 0;
    g_SwapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    g_SwapChainDescriptor.Windowed = true;

    if (FAILED(g_Factory->CreateSwapChain(g_Device.Get(), &g_SwapChainDescriptor, &g_SwapChain)))
    {
        std::cout << "DXGI: Unable to create swapchain\n";
        return false;
    }

    ID3D11Texture2D* backBuffer = nullptr;;
    if (FAILED(g_SwapChain->GetBuffer(
        0,
        IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "DXGI: Unable to retrieve backbuffer from swapchain\n";
        return false;
    }

    if (FAILED(g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_BackBufferRtv)))
    {
        std::cout << "D3D11: Unable to create RTV from backbuffer\n";
        backBuffer->Release();
        return false;
    }

    backBuffer->Release();

    D3D11_TEXTURE2D_DESC depthStencilBufferDescriptor = {};
    depthStencilBufferDescriptor.ArraySize = 1;
    depthStencilBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDescriptor.CPUAccessFlags = 0;
    depthStencilBufferDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDescriptor.Width = windowWidth;
    depthStencilBufferDescriptor.Height = windowHeight;
    depthStencilBufferDescriptor.MipLevels = 1;
    depthStencilBufferDescriptor.SampleDesc.Count = 1;
    depthStencilBufferDescriptor.SampleDesc.Quality = 0;
    depthStencilBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    if (FAILED(g_Device->CreateTexture2D(&depthStencilBufferDescriptor, nullptr, &g_DepthStencilBuffer)))
    {
        std::cout << "D3D11: Unable to create depth stencil buffer\n";
        return false;
    }

    if (FAILED(g_Device->CreateDepthStencilView(g_DepthStencilBuffer.Get(), nullptr, &g_DepthStencilView)))
    {
        std::cout << "D3D11: Unable to create depth stencil view\n";
        return false;
    }

    g_ViewPort.Width = static_cast<float>(windowWidth);
    g_ViewPort.Height = static_cast<float>(windowHeight);
    g_ViewPort.TopLeftX = 0.0f;
    g_ViewPort.TopLeftY = 0.0f;
    g_ViewPort.MinDepth = 0.0f;
    g_ViewPort.MaxDepth = 1.0f;

    if (FAILED(CoInitialize(nullptr)))
    {
        std::cout << "WIN32: Unable to initialize COM library\n";
        return false;
    }
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_ImagingFactory))))
    {
        std::cout << "WIC: Unable to create imaging factory\n";
        return false;
    }


    return true;
}

bool LoadFile(const std::string_view filePath, std::vector<std::uint8_t>& fileContent)
{
    std::ifstream fileStream(filePath.data(), std::ios::in | std::ios::binary);
    if (!fileStream.is_open())
    {
        std::cout << "IO: Unable to open file " << filePath.data() << "\n";
        return false;
    }

    std::cout << "IO: Loading file from " << std::string(filePath.data()).c_str() << "\n";
    fileContent = std::vector<uint8_t>(std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>());
    return true;
}

ID3D11VertexShader* LoadVertexShader(const std::wstring_view filePath, ID3DBlob** vertexShaderBlob)
{
    if (FAILED(D3DReadFileToBlob(filePath.data(), vertexShaderBlob)))
    {
        std::cout << "D3D11: Unable to create vertex shader from file " << std::wstring(filePath.data()).c_str() << "\n";
        return nullptr;
    }

    ID3D11VertexShader* vertexShader;
    if (FAILED(g_Device->CreateVertexShader(
        (*vertexShaderBlob)->GetBufferPointer(),
        (*vertexShaderBlob)->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        std::cout << "D3D11: Unable to create vertex shader from file " << std::wstring(filePath.data()).c_str() << "\n";
        return nullptr;
    }

    return vertexShader;
}

ID3D11PixelShader* LoadPixelShader(const std::wstring_view filePath)
{
    ID3DBlob* pixelShaderBlob;
    if (FAILED(D3DReadFileToBlob(filePath.data(), &pixelShaderBlob)))
    {
        std::cout << "D3D11: Unable to create vertex shader from file " << std::wstring(filePath.data()).c_str() << "\n";
        return nullptr;
    }

    ID3D11PixelShader* vertexShader;
    if (FAILED(g_Device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        std::cout << "D3D11: Unable to create vertex shader from file " << std::wstring(filePath.data()).c_str() << "\n";
        return nullptr;
    }

    pixelShaderBlob->Release();

    return vertexShader;
}

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

DXGI_FORMAT GetTextureFormat(const WICPixelFormatGUID pixelFormat)
{
    static std::unordered_map<WICPixelFormatGUID, DXGI_FORMAT> pixelFormatToTextureFormat =
    {
        {GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT},
        {GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT},
        {GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UNORM},
        {GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM},
        {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM},
        {GUID_WICPixelFormat32bppBGR, DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM},
        {GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM},
        {GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UNORM},
        {GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT::DXGI_FORMAT_B5G5R5A1_UNORM},
        {GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT::DXGI_FORMAT_B5G6R5_UNORM},
        {GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT},
        {GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT},
        {GUID_WICPixelFormat16bppGray, DXGI_FORMAT::DXGI_FORMAT_R16_UNORM},
        {GUID_WICPixelFormat8bppGray, DXGI_FORMAT::DXGI_FORMAT_R8_UNORM},
        {GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT::DXGI_FORMAT_A8_UNORM},
    };

    if (pixelFormatToTextureFormat.find(pixelFormat) != pixelFormatToTextureFormat.end())
    {
        return pixelFormatToTextureFormat[pixelFormat];
    }

    return DXGI_FORMAT_UNKNOWN;
}

std::int8_t GetBitsPerPixel(const DXGI_FORMAT& textureFormat)
{
    assert(textureFormat != DXGI_FORMAT_UNKNOWN);

    static std::unordered_map<DXGI_FORMAT, std::int8_t> textureFormatToBitsPerPixel =
    {
        {DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 128},
        {DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 64},
        {DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UNORM, 64},
        {DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 32},
        {DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, 32},
        {DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM, 32},
        {DXGI_FORMAT::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, 32},
        {DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UNORM, 32},
        {DXGI_FORMAT::DXGI_FORMAT_B5G5R5A1_UNORM, 16},
        {DXGI_FORMAT::DXGI_FORMAT_B5G6R5_UNORM, 16},
        {DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, 32},
        {DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 16},
        {DXGI_FORMAT::DXGI_FORMAT_R16_UNORM, 16},
        {DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, 8},
        {DXGI_FORMAT::DXGI_FORMAT_A8_UNORM, 16},
    };

    return textureFormatToBitsPerPixel[textureFormat];
}

bool LoadTexture(IWICImagingFactory* imagingFactory, const std::wstring_view filePath, ID3D11ShaderResourceView** textureView)
{
    IWICBitmapDecoder* bitmapDecoder = nullptr;
    IWICBitmapFrameDecode* bitmapFrame = nullptr;
    IWICFormatConverter* formatConverter = nullptr;

    if (FAILED(imagingFactory->CreateDecoderFromFilename(
        filePath.data(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &bitmapDecoder)))
    {
        std::cout << "WIC: Unable to create image decoder from file " << std::wstring(filePath.data()).c_str() << "\n";
        return false;
    }

    if (FAILED(bitmapDecoder->GetFrame(0, &bitmapFrame)))
    {
        bitmapDecoder->Release();

        std::cout << "WIC: Unable to get image frame\n";
        return false;
    }

    WICPixelFormatGUID pixelFormat;
    if (FAILED(bitmapFrame->GetPixelFormat(&pixelFormat)))
    {
        bitmapFrame->Release();

        std::cout << "WIC: Unable to get the pixelformat\n";
        return false;
    }

    UINT imageWidth = 0;
    UINT imageHeight = 0;
    if (FAILED(bitmapFrame->GetSize(&imageWidth, &imageHeight)))
    {
        bitmapFrame->Release();

        std::cout << "WIC: Unable to get the image size\n";
        return false;
    }

    const auto textureFormat = GetTextureFormat(pixelFormat);
    const auto bitsPerPixel = GetBitsPerPixel(textureFormat);
    const auto bytesPerRow = (imageWidth * bitsPerPixel) / 8;
    const auto pixelDataSize = bytesPerRow * imageHeight;

    std::vector<BYTE> pixelData(pixelDataSize);

    if (FAILED(bitmapFrame->CopyPixels(nullptr, bytesPerRow, pixelDataSize, pixelData.data())))
    {
        bitmapFrame->Release();

        std::cout << "WIC: Unable to copy pixels\n";
        return false;
    }

    D3D11_TEXTURE2D_DESC textureDescriptor = {};
    textureDescriptor.ArraySize = 1;
    textureDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    textureDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
    textureDescriptor.Format = textureFormat;
    textureDescriptor.MipLevels = 1;// +floor(log2(DirectX::XMMax(imageWidth, imageHeight)));
    textureDescriptor.Height = imageHeight;
    textureDescriptor.Width = imageWidth;
    textureDescriptor.SampleDesc.Quality = 0;
    textureDescriptor.SampleDesc.Count = 1;
    textureDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;

    const D3D11_SUBRESOURCE_DATA textureData = { pixelData.data(), bytesPerRow, 0 };

    ID3D11Texture2D* texture = nullptr;
    if (FAILED(g_Device->CreateTexture2D(&textureDescriptor, &textureData, &texture)))
    {
        std::cout << "D3D11: Unable to create a texture for file " << std::wstring(filePath.data()).c_str() << "\n";
        return false;
    }

    if (FAILED(g_Device->CreateShaderResourceView(texture, nullptr, textureView)))
    {
        texture->Release();

        std::cout << "D3D11: Unable to create a shader resource view from the texture\n";
        return false;
    }

    texture->Release();
    bitmapFrame->Release();
    bitmapDecoder->Release();
    return true;
}

bool Load()
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
    depthStencilDescriptor.DepthEnable = true;
    depthStencilDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
    depthStencilDescriptor.StencilEnable = false;
    if (FAILED(g_Device->CreateDepthStencilState(&depthStencilDescriptor, &g_DepthStencilState)))
    {
        std::cout << "D3D11: Unable to create depth stencil state\n";
        return false;
    }

    D3D11_RASTERIZER_DESC rasterizerDescriptor = {};
    rasterizerDescriptor.AntialiasedLineEnable = false;
    rasterizerDescriptor.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rasterizerDescriptor.DepthBias = 0;
    rasterizerDescriptor.DepthBiasClamp = 0.0f;
    rasterizerDescriptor.DepthClipEnable = true;
    rasterizerDescriptor.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rasterizerDescriptor.FrontCounterClockwise = false;
    rasterizerDescriptor.MultisampleEnable = false;
    rasterizerDescriptor.ScissorEnable = false;
    rasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
    if (FAILED(g_Device->CreateRasterizerState(&rasterizerDescriptor, &g_RasterizerState)))
    {
        std::cout << "D3D11: Unable to create rasterizer state\n";
        return false;
    }

    g_VertexShader = LoadVertexShader(L"Assets/Shaders/SimpleShader.vs.cso", &g_VertexShaderBlob);
    if (g_VertexShader == nullptr)
    {
        return false;
    }

    g_PixelShader = LoadPixelShader(L"Assets/Shaders/SimpleShader.ps.cso");
    if (g_PixelShader == nullptr)
    {
        return false;
    }

    const D3D11_BUFFER_DESC vertexBufferDescriptor = CD3D11_BUFFER_DESC(
        sizeof(VertexPositionColorUv) * _countof(g_Vertices),
        D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER);
    constexpr D3D11_SUBRESOURCE_DATA vertexBufferData = { g_Vertices, 0, 0 };
    if (FAILED(g_Device->CreateBuffer(&vertexBufferDescriptor, &vertexBufferData, &g_VertexBuffer)))
    {
        std::cout << "D3D11: Unable to create vertex buffer\n";
        return false;
    }

    const D3D11_BUFFER_DESC indexBufferDescriptor = CD3D11_BUFFER_DESC(
        sizeof(std::uint16_t) * _countof(g_Indices),
        D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER);
    constexpr D3D11_SUBRESOURCE_DATA indexBufferData = { g_Indices, 0, 0 };
    if (FAILED(g_Device->CreateBuffer(&indexBufferDescriptor, &indexBufferData, &g_IndexBuffer)))
    {
        std::cout << "D3D11: Unable to create index buffer\n";
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC vertexPositionColorLayoutDescriptor[] =
    {
        { "POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPositionColorUv, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPositionColorUv, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPositionColorUv, Uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    if (FAILED(g_Device->CreateInputLayout(
        vertexPositionColorLayoutDescriptor,
        _countof(vertexPositionColorLayoutDescriptor),
        g_VertexShaderBlob->GetBufferPointer(),
        g_VertexShaderBlob->GetBufferSize(),
        &g_InputLayout)))
    {
        std::cout << "D3D11: Unable to create input layout\n";
        return false;
    }

    g_VertexShaderBlob->Release();
    g_VertexShaderBlob = nullptr;

    const D3D11_BUFFER_DESC constantBufferDescriptor = CD3D11_BUFFER_DESC(
        sizeof(DirectX::XMMATRIX),
        D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER);
    if (FAILED(g_Device->CreateBuffer(&constantBufferDescriptor, nullptr, &g_ConstantBuffers[0])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerApplication\n";
        return false;
    }
    if (FAILED(g_Device->CreateBuffer(&constantBufferDescriptor, nullptr, &g_ConstantBuffers[1])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerFrame\n";
        return false;
    }
    if (FAILED(g_Device->CreateBuffer(&constantBufferDescriptor, nullptr, &g_ConstantBuffers[2])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerObject\n";
        return false;
    }

    D3D11_SAMPLER_DESC linearSamplerStateDescriptor = {};
    linearSamplerStateDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    linearSamplerStateDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    linearSamplerStateDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    linearSamplerStateDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    if (FAILED(g_Device->CreateSamplerState(&linearSamplerStateDescriptor, &g_LinearSamplerState)))
    {
        std::cout << "D3D11: Unable to create linear sampler state\n";
        return false;
    }

    if (!LoadTexture(g_ImagingFactory, L"Assets/Textures/T_Froge.png", &g_TextureSrv))
    {
        return false;
    }

    return true;
}

void Update()
{
    const auto eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
    const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    g_ViewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    g_DeviceContext->UpdateSubresource(g_ConstantBuffers[PerFrame].Get(), 0, nullptr, &g_ViewMatrix, 0, 0);

    static float angle = 0.0f;
    angle += 90.0f * (1.0 / 60000.0f);
    const auto rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);

    g_WorldMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
    g_DeviceContext->UpdateSubresource(g_ConstantBuffers[PerObject].Get(), 0, nullptr, &g_WorldMatrix, 0, 0);
}

void Render()
{
    constexpr UINT vertexBufferOffset = 0;

    g_DeviceContext->ClearRenderTargetView(g_BackBufferRtv.Get(), g_ClearColor);
    g_DeviceContext->ClearDepthStencilView(g_DepthStencilView.Get(), D3D10_CLEAR_FLAG::D3D10_CLEAR_DEPTH, 1.0, 0);

    g_DeviceContext->IASetInputLayout(g_InputLayout.Get());
    g_DeviceContext->IASetIndexBuffer(g_IndexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R16_UINT, 0);
    g_DeviceContext->IASetVertexBuffers(0, 1, g_VertexBuffer.GetAddressOf(), &g_VertexBufferStride, &vertexBufferOffset);
    g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_DeviceContext->VSSetShader(g_VertexShader.Get(), nullptr, 0);
    g_DeviceContext->VSSetConstantBuffers(0, 3, g_ConstantBuffers->GetAddressOf());

    g_DeviceContext->RSSetState(g_RasterizerState.Get());
    g_DeviceContext->RSSetViewports(1, &g_ViewPort);

    g_DeviceContext->PSSetShader(g_PixelShader.Get(), nullptr, 0);
    g_DeviceContext->PSSetShaderResources(0, 1, g_TextureSrv.GetAddressOf());

    ID3D11SamplerState* samplers[] = { g_LinearSamplerState.Get() };
    g_DeviceContext->PSSetSamplers(0, static_cast<UINT>(std::size(samplers)), samplers);

    g_DeviceContext->OMSetRenderTargets(1, g_BackBufferRtv.GetAddressOf(), g_DepthStencilView.Get());
    g_DeviceContext->OMSetDepthStencilState(g_DepthStencilState.Get(), 1);
    g_DeviceContext->DrawIndexed(_countof(g_Indices), 0, 0);

    g_SwapChain->Present(0, 0);
}

void Cleanup()
{
    g_DepthStencilView.Reset();
    g_DepthStencilBuffer.Reset();
    g_BackBufferRtv.Reset();

    g_RasterizerState.Reset();
    g_DepthStencilState.Reset();
    g_LinearSamplerState.Reset();

    for (auto i = 0; i < NumConstantBufferTypes; i++)
    {
        g_ConstantBuffers[i].Reset();
    }
    g_VertexShader.Reset();
    g_PixelShader.Reset();
    g_VertexBuffer.Reset();
    g_IndexBuffer.Reset();
    g_InputLayout.Reset();
    g_SwapChain.Reset();

    g_DeviceContext.Reset();

    g_Debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_IGNORE_INTERNAL);
    g_Debug.Reset();

    g_Device.Reset();

    glfwDestroyWindow(g_Window);
    glfwTerminate();
}

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (!Initialize())
    {
        Cleanup();
        return -1;
    }

    if (!Load())
    {
        Cleanup();
        return -2;
    }

    std::int32_t windowWidth = 0;
    std::int32_t windowHeight = 0;
    glfwGetWindowSize(g_Window, &windowWidth, &windowHeight);
    g_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        windowWidth / static_cast<float>(windowHeight),
        0.1f,
        128.0f);
    g_DeviceContext->UpdateSubresource(g_ConstantBuffers[PerApplication].Get(), 0, nullptr, &g_ProjectionMatrix, 0, 0);

    while (!glfwWindowShouldClose(g_Window))
    {
        glfwPollEvents();

        Update();
        Render();
    }

    Cleanup();
    CoUninitialize();

    return 0;
}
