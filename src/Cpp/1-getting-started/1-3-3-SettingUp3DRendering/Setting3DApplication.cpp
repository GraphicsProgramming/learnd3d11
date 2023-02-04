#include "Setting3DApplication.hpp"
#include "ShaderCollection.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <DirectXTex.h>
#include <FreeImage.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

#if defined(_DEBUG)
#pragma comment(lib, "FreeImageLibd.lib")
#else
#pragma comment(lib, "FreeImageLib.lib")
#endif

template <UINT TDebugNameLength>
inline void SetDebugName(_In_ ID3D11DeviceChild* deviceResource, _In_z_ const char (&debugName)[TDebugNameLength])
{
    deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
}

Setting3DApplication::Setting3DApplication(const std::string& title)
    : Application(title)
{
}

Setting3DApplication::~Setting3DApplication()
{
    _deviceContext->Flush();
    _textureSrv.Reset();
    _triangleVertices.Reset();
    _perFrameConstantBuffer.Reset();
    _perObjectConstantBuffer.Reset();
    _linearSamplerState.Reset();
    _rasterState.Reset();
    _shaderCollection.Destroy();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.Reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
}

bool Setting3DApplication::Initialize()
{
    if (!Application::Initialize())
    {
        return false;
    }

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

#if !defined(NDEBUG)
    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }
#endif

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
        std::cout << "DXGI: Failed to create SwapChain\n";
        return false;
    }

    CreateSwapchainResources();

    CreateConstantBuffers();

    FreeImage_Initialise();
    return true;
}

WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureViewFromDDS(ID3D11Device* device, const std::wstring& pathToDDS)
{
    DirectX::TexMetadata metaData = {};
    DirectX::ScratchImage scratchImage;
    if (FAILED(DirectX::LoadFromDDSFile(pathToDDS.c_str(), DirectX::DDS_FLAGS_NONE, &metaData, scratchImage)))
    {
        std::cout << "DXTEX: Failed to load image\n";
        return nullptr;
    }

    WRL::ComPtr<ID3D11Resource> texture = nullptr;
    if (FAILED(DirectX::CreateTexture(
            device,
            scratchImage.GetImages(),
            scratchImage.GetImageCount(),
            metaData,
            &texture)))
    {
        std::cout << "DXTEX: Failed to create texture out of image\n";
        scratchImage.Release();
        return nullptr;
    }

    ID3D11ShaderResourceView* srv = nullptr;

    if (FAILED(DirectX::CreateShaderResourceView(
            device,
            scratchImage.GetImages(),
            scratchImage.GetImageCount(),
            metaData,
            &srv)))
    {
        std::cout << "DXTEX: Failed to create shader resource view out of texture\n";
        scratchImage.Release();
        return nullptr;
    }

    return srv;
}


WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureView(ID3D11Device* device, const std::wstring& pathToTexture)
{
    FIBITMAP* image = nullptr; 
    //Win32 methods of opening files is called "CreateFile" counterintuitively, we make sure to tell it to only to read pre-existing files
    HANDLE file = CreateFileW(pathToTexture.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, 0);

    //If the file didn't exist we'll get an invalid handle
    if (file == INVALID_HANDLE_VALUE)
    {
        return nullptr;
    }

    size_t fileSize = GetFileSize(file, nullptr);

    //We open a new local scope here so we don't keep the vector in memory for the entire function call, we can get rid of the memory it holds earlier this way
    {
        std::vector<BYTE> fileDataRaw(fileSize);
        if (!ReadFile(file, fileDataRaw.data(), static_cast<DWORD>(fileDataRaw.size()), nullptr, nullptr))
        {
            CloseHandle(file);
            return nullptr;
        }

        //Close our file handle as we don't need it anymore
        CloseHandle(file);

        FIMEMORY* memHandle = FreeImage_OpenMemory(fileDataRaw.data(), static_cast<DWORD>(fileDataRaw.size()));
        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeFromMemory(memHandle);
        if (imageFormat == FIF_UNKNOWN)
        {
            FreeImage_CloseMemory(memHandle);
            std::cout << "CreateTextureView: Unsupported texture format from file: '" << pathToTexture.c_str() << "'\n";
            return nullptr;
        }
        image = FreeImage_LoadFromMemory(imageFormat, memHandle);

        //We no longer need the original data
        FreeImage_CloseMemory(memHandle);

    } //ending the local scope cleans up fileDataRaw

    //Flip the image vertically so this matches up with what DirectXTex loads
    FreeImage_FlipVertical(image);

    uint32_t textureWidth = FreeImage_GetWidth(image);
    uint32_t textureHeight = FreeImage_GetHeight(image);
    uint32_t textureBPP = FreeImage_GetBPP(image);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    D3D11_SUBRESOURCE_DATA initialData = {};
    WRL::ComPtr<ID3D11Texture2D> texture = nullptr;

    DXGI_FORMAT textureFormat;
    switch (textureBPP)
    {
    case 8:
        textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8_UNORM;
        break;
    case 16:
        textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM;
        break;
    case 24:
        //D3D11 does not support 24 bit formats for textures, we'll need to convert
        {
            textureBPP = 32;
            FIBITMAP* newImage = FreeImage_ConvertTo32Bits(image);
            FreeImage_Unload(image);
            image = newImage;
            textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        break;
    case 32:
        textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        {
            //we could try to handle some weird bitcount, but these will probably be HDR or some antique format, just exit instead..
            std::cout << "CreateTextureView: Texture has nontrivial bits per pixel ( " << textureBPP << " ), file: '" << pathToTexture.c_str() << "'\n";
            return nullptr;
        }
        break;
    }
    textureDesc.Format = textureFormat;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Height = textureHeight;
    textureDesc.Width = textureWidth;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    //populate initial data
    initialData.pSysMem = FreeImage_GetBits(image);
    initialData.SysMemPitch = (textureBPP / 8) * textureWidth;

    if (FAILED(device->CreateTexture2D(&textureDesc, &initialData, texture.GetAddressOf())))
    {
        FreeImage_Unload(image);
        return nullptr;
    }
    FreeImage_Unload(image);

    ID3D11ShaderResourceView* srv = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = textureDesc.Format;
    srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

    if (FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv)))
    {
        std::cout << "CreateTextureView: Failed to create SRV from texture: " << pathToTexture.c_str() << "\n";
        return nullptr;
    }

    return srv;
}

void Setting3DApplication::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = sizeof(PerFrameConstantBuffer);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    _device->CreateBuffer(&desc, nullptr, &_perFrameConstantBuffer);

    desc.ByteWidth = sizeof(PerObjectConstantBuffer);
    _device->CreateBuffer(&desc, nullptr, &_perObjectConstantBuffer);
}

bool Setting3DApplication::Load()
{
    ShaderCollectionDescriptor shaderDescriptor = {};
    shaderDescriptor.VertexShaderFilePath = L"Assets/Shaders/Main.vs.hlsl";
    shaderDescriptor.PixelShaderFilePath = L"Assets/Shaders/Main.ps.hlsl";
    shaderDescriptor.VertexType = VertexType::PositionColorUv;

    _shaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

    constexpr VertexPositionColorUv vertices[] = {
        {Position{  0.0f,  0.5f, 0.0f }, Color{ 0.25f, 0.39f, 0.19f }, Uv{ 0.5f, 0.0f }},
        {Position{  0.5f, -0.5f, 0.0f }, Color{ 0.44f, 0.75f, 0.35f }, Uv{ 1.0f, 1.0f }},
        {Position{ -0.5f, -0.5f, 0.0f }, Color{ 0.38f, 0.55f, 0.20f }, Uv{ 0.0f, 1.0f }},
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

    _fallbackTextureSrv = CreateTextureView(_device.Get(), L"Assets/Textures/default.png");
    assert(_fallbackTextureSrv != nullptr); //as a fallback resource, this "needs" to exist

    _textureSrv = CreateTextureViewFromDDS(_device.Get(), L"Assets/Textures/T_Froge.dds");
    if (_textureSrv == nullptr)
    {
        //this is "fine", we can use our fallback!
        _textureSrv = _fallbackTextureSrv;
    }


    D3D11_SAMPLER_DESC linearSamplerStateDescriptor = {};
    linearSamplerStateDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    linearSamplerStateDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    linearSamplerStateDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    linearSamplerStateDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    if (FAILED(_device->CreateSamplerState(&linearSamplerStateDescriptor, &_linearSamplerState)))
    {
        std::cout << "D3D11: Failed to create linear sampler state\n";
        return false;
    }

    D3D11_RASTERIZER_DESC rasterDesc{};
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FillMode = D3D11_FILL_SOLID;

    _device->CreateRasterizerState(&rasterDesc, &_rasterState);

    return true;
}

bool Setting3DApplication::CreateSwapchainResources()
{
    WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    if (FAILED(_swapChain->GetBuffer(
            0,
            IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get back buffer from swapchain\n";
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

void Setting3DApplication::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void Setting3DApplication::OnResize(
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

void Setting3DApplication::Update()
{
    Application::Update();

    using namespace DirectX;

    static float _yRotation = 0.0f;
    static float _scale = 1.0f;
    static XMFLOAT3 _cameraPosition = {0.0f, 0.0f, -1.0f};


    _yRotation += _deltaTime;

    //////////////////////////
    //This will be our "camera"
    XMVECTOR camPos = XMLoadFloat3(&_cameraPosition);

    XMMATRIX view = XMMatrixLookAtLH(camPos, g_XMZero, { 0,1,0,1 });
    XMMATRIX proj = XMMatrixPerspectiveFovLH(90.0f * 0.0174533f,
                                            static_cast<float>(_width) / static_cast<float>(_height),
                                            0.1f,
                                            100.0f);
    //combine the view & proj matrix
    XMMATRIX viewProjection = XMMatrixMultiply(view, proj);
    XMStoreFloat4x4(&_perFrameConstantBufferData.viewProjectionMatrix, viewProjection);
    //////////////////////////

    //////////////////////////
    //This will define our 3D object
    XMMATRIX translation = XMMatrixTranslation(0, 0, 0);
    XMMATRIX scaling = XMMatrixScaling(_scale, _scale, _scale);
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(0, _yRotation, 0);

    //Now we create our model matrix
    XMMATRIX modelMatrix = XMMatrixMultiply(translation, XMMatrixMultiply(scaling, rotation));
    XMStoreFloat4x4(&_perObjectConstantBufferData.modelMatrix, modelMatrix);
    /////////////////////////

    /////////////////////////
    //Update our constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    _deviceContext->Map(_perFrameConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &_perFrameConstantBufferData, sizeof(PerFrameConstantBuffer));
    _deviceContext->Unmap(_perFrameConstantBuffer.Get(), 0);

    _deviceContext->Map(_perObjectConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &_perObjectConstantBufferData, sizeof(PerObjectConstantBuffer));
    _deviceContext->Unmap(_perObjectConstantBuffer.Get(), 0);
    /////////////////////////
}

void Setting3DApplication::Render()
{
    float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    constexpr UINT vertexOffset = 0;
    ID3D11RenderTargetView* nullTarget = nullptr;

    //set to 0 so we can clear properly
    _deviceContext->OMSetRenderTargets(1, &nullTarget, nullptr);
    _deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);

    _deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_BUFFER_DESC description = {};
    _triangleVertices->GetDesc(&description);
    UINT stride = _shaderCollection.GetLayoutByteSize(VertexType::PositionColorUv);
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices.GetAddressOf(),
        &stride,
        &vertexOffset);

    _shaderCollection.ApplyToContext(_deviceContext.Get());

    D3D11_VIEWPORT viewport = {
        0.0f,
        0.0f,
        static_cast<float>(GetWindowWidth()),
        static_cast<float>(GetWindowHeight()),
        0.0f,
        1.0f
    };

    _deviceContext->RSSetViewports(1, &viewport);
    _deviceContext->RSSetState(_rasterState.Get());


    _deviceContext->PSSetShaderResources(0, 1, _textureSrv.GetAddressOf());
    _deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

    ID3D11Buffer* constantBuffers[2] =
    {
        _perFrameConstantBuffer.Get(),
        _perObjectConstantBuffer.Get()
    };

    _deviceContext->VSSetConstantBuffers(0, 2, constantBuffers);
    _deviceContext->Draw(3,0);
    _swapChain->Present(1, 0);
}
