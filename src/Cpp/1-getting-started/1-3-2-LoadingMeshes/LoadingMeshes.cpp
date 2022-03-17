#include "LoadingMeshes.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3dcompiler.h>

#undef min
#undef max

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <vector>

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

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;
using Uv = DirectX::XMFLOAT2;

struct VertexPositionColorUv
{
    Position position;
    Color color;
    Uv uv;
};

LoadingMeshesApplication::LoadingMeshesApplication(const std::string_view title)
    : Application(title)
{
}

LoadingMeshesApplication::~LoadingMeshesApplication()
{
    _deviceContext->Flush();
    _constantBuffers[ConstantBufferType::PerApplication].Reset();
    _constantBuffers[ConstantBufferType::PerFrame].Reset();
    _constantBuffers[ConstantBufferType::PerObject].Reset();
    _textureSrv.Reset();
    _pixelShader.Reset();
    _vertexShader.Reset();
    _vertexLayout.Reset();
    _modelVertices.Reset();
    _modelIndices.Reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.Reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
    Application::Cleanup();
}

bool LoadingMeshesApplication::Initialize()
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

    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }

    _textureFactory = std::make_unique<TextureFactory>(_device);

    constexpr char deviceName[] = "DEV_Main";
    _device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(_deviceContext.Get(), "CTX_Main");

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

    _shaderFactory = std::make_unique<ShaderFactory>(_device);

    WRL::ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
    _vertexShader = _shaderFactory->CreateVertexShader(L"Assets/Shaders/Main.vs.hlsl", vertexShaderBlob);
    if (_vertexShader == nullptr)
    {
        return false;
    }

    _pixelShader = _shaderFactory->CreatePixelShader(L"Assets/Shaders/Main.ps.hlsl");
    if (_pixelShader == nullptr)
    {
        return false;
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexPositionColorUv, position),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexPositionColorUv, color),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,
            0,
            offsetof(VertexPositionColorUv, uv),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };
    if (FAILED(_device->CreateInputLayout(
        vertexInputLayoutInfo,
        _countof(vertexInputLayoutInfo),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &_vertexLayout)))
    {
        std::cout << "D3D11: Failed to create default vertex input layout\n";
        return false;
    }

    if (!_textureFactory->CreateShaderResourceViewFromFile(L"Assets/Textures/T_Good_Froge.dds", _textureSrv))
    {
        return false;
    }

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

    if (!LoadModel(L"Assets/Models/SM_Good_Froge.fbx"))
    {
        return false;
    }

    const D3D11_BUFFER_DESC constantBufferDescriptor = CD3D11_BUFFER_DESC(
        sizeof(DirectX::XMMATRIX),
        D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER);

    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerApplication])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerApplication\n";
        return false;
    }
    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerFrame])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerFrame\n";
        return false;
    }
    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerObject])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerObject\n";
        return false;
    }

    _projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        GetWindowWidth() / static_cast<float>(GetWindowHeight()),
        0.1f,
        512.0f);
    _deviceContext->UpdateSubresource(_constantBuffers[PerApplication].Get(), 0, nullptr, &_projectionMatrix, 0, 0);

    return true;
}

bool LoadingMeshesApplication::CreateSwapchainResources()
{
    WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
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

void LoadingMeshesApplication::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void LoadingMeshesApplication::OnResize(
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

    _projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        GetWindowWidth() / static_cast<float>(GetWindowHeight()),
        0.1f,
        512);
    _deviceContext->UpdateSubresource(_constantBuffers[PerApplication].Get(), 0, nullptr, &_projectionMatrix, 0, 0);
}

void LoadingMeshesApplication::Update()
{
    const auto eyePosition = DirectX::XMVectorSet(0, 50, 200, 1);
    const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    _viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    _deviceContext->UpdateSubresource(_constantBuffers[PerFrame].Get(), 0, nullptr, &_viewMatrix, 0, 0);

    static float angle = 0.0f;
    angle += 90.0f * (10.0 / 60000.0f);
    const auto rotationAxis = DirectX::XMVectorSet(0, 1, 0, 0);

    _worldMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
    _deviceContext->UpdateSubresource(_constantBuffers[PerObject].Get(), 0, nullptr, &_worldMatrix, 0, 0);
}

void LoadingMeshesApplication::Render()
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(GetWindowWidth());
    viewport.Height = static_cast<float>(GetWindowHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

    constexpr uint32_t vertexStride = sizeof(VertexPositionColorUv);
    constexpr uint32_t vertexOffset = 0;

    _deviceContext->ClearRenderTargetView(
        _renderTarget.Get(),
        clearColor);

    _deviceContext->IASetInputLayout(_vertexLayout.Get());
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _modelVertices.GetAddressOf(),
        &vertexStride,
        &vertexOffset);
    _deviceContext->IASetIndexBuffer(_modelIndices.Get(), DXGI_FORMAT_R32_UINT, 0);
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _deviceContext->VSSetShader(
        _vertexShader.Get(),
        nullptr,
        0);
    _deviceContext->VSSetConstantBuffers(0, 3, _constantBuffers->GetAddressOf());

    _deviceContext->RSSetViewports(
        1,
        &viewport);

    _deviceContext->PSSetShader(
        _pixelShader.Get(),
        nullptr,
        0);
    _deviceContext->PSSetShaderResources(0, 1, _textureSrv.GetAddressOf());
    _deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());
    _deviceContext->OMSetRenderTargets(
        1,
        _renderTarget.GetAddressOf(),
        nullptr);

    _deviceContext->DrawIndexed(_modelIndexCount, 0, 0);
    _swapChain->Present(1, 0);
}

bool LoadingMeshesApplication::LoadModel(const std::wstring_view filePath)
{
    constexpr uint32_t importFlags = aiProcess_Triangulate | aiProcess_FlipUVs;
    const std::string fileName{ filePath.begin(), filePath.end() };

    Assimp::Importer sceneImporter;
    const aiScene* scene = sceneImporter.ReadFile(fileName.c_str(), importFlags);

    if (scene == nullptr)
    {
        std::cout << "ASSIMP: Unable to load model file\n";
        return false;
    }

    if (!scene->HasMeshes())
    {
        std::cout << "ASSIMP: Model file is empty\n";
        return false;
    }

    const aiMesh* mesh = scene->mMeshes[0];
    if (!mesh->HasPositions())
    {
        std::cout << "ASSIMP: Model mesh has no positions\n";
        return false;
    }

    constexpr Color defaultColor = Color{ 0.5f, 0.5f, 0.5f };
    constexpr Uv defaultUv = Uv{ 0.0f, 0.0f };
    std::vector<VertexPositionColorUv> vertices;
    for (int32_t i = 0; i < (mesh->mNumVertices); i++)
    {
        const Position& position = Position{ mesh->mVertices[i].x / 1.0f, mesh->mVertices[i].y / 1.0f, mesh->mVertices[i].z / 1.0f };
        const Color& color = mesh->HasVertexColors(0)
            ? Color{ mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b}
            : defaultColor;
        const Uv& uv = mesh->HasTextureCoords(0)
            ? Uv{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }
            : defaultUv;

        vertices.push_back(VertexPositionColorUv{ Position{position}, Color{color}, Uv{uv} });
    }

    _modelVertexCount = vertices.size();

    D3D11_BUFFER_DESC vertexBufferDescriptor = {};
    vertexBufferDescriptor.ByteWidth = static_cast<uint32_t>(sizeof(VertexPositionColorUv) * vertices.size());
    vertexBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    vertexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertexBufferData = {};
    vertexBufferData.pSysMem = vertices.data();
    if (FAILED(_device->CreateBuffer(
        &vertexBufferDescriptor,
        &vertexBufferData,
        &_modelVertices)))
    {
        std::cout << "D3D11: Failed to create model vertex buffer\n";
        return false;
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        for (uint32_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    _modelIndexCount = indices.size();

    D3D11_BUFFER_DESC indexBufferDescriptor = {};
    indexBufferDescriptor.ByteWidth = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());
    indexBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    indexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA indexBufferData = {};
    indexBufferData.pSysMem = indices.data();
    if (FAILED(_device->CreateBuffer(
        &indexBufferDescriptor,
        &indexBufferData,
        &_modelIndices)))
    {
        std::cout << "D3D11: Failed to create model index buffer\n";
        return false;
    }

    return true;
}
