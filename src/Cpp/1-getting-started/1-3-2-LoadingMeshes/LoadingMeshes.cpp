#include "LoadingMeshes.hpp"
#include "DeviceContext.hpp"
#include "PipelineFactory.hpp"
#include "TextureFactory.hpp"
#include "Pipeline.hpp"

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
    _pipeline.reset();
    _pipelineFactory.reset();
    _modelVertices.Reset();
    _modelIndices.Reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.reset
    ();
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
    _textureFactory = std::make_unique<TextureFactory>(_device);

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
    if (!_pipelineFactory->CreatePipeline(pipelineSettings, _pipeline))
    {
        std::cout << "PipelineFactory: Unable to create pipeline\n";
        return false;
    }

    if (!_textureFactory->CreateShaderResourceViewFromFile(L"Assets/Textures/T_Good_Froge.dds", _textureSrv))
    {
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

    if (!LoadModel("Assets/Models/SM_Good_Froge.fbx"))
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
    _pipeline->BindVertexStageConstantBuffer(0, _constantBuffers[ConstantBufferType::PerApplication].Get());
    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerFrame])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerFrame\n";
        return false;
    }
    _pipeline->BindVertexStageConstantBuffer(1, _constantBuffers[ConstantBufferType::PerFrame].Get());
    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerObject])))
    {
        std::cout << "D3D11: Unable to create constant buffer PerObject\n";
        return false;
    }
    _pipeline->BindVertexStageConstantBuffer(2, _constantBuffers[ConstantBufferType::PerObject].Get());

    _projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        GetWindowWidth() / static_cast<float>(GetWindowHeight()),
        0.1f,
        512.0f);
    _deviceContext->UpdateSubresource(_constantBuffers[PerApplication].Get(), &_projectionMatrix);

    return true;
}

bool LoadingMeshesApplication::CreateSwapchainResources()
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
    _deviceContext->UpdateSubresource(_constantBuffers[ConstantBufferType::PerApplication].Get(), &_projectionMatrix);
}

void LoadingMeshesApplication::Update()
{
    const auto eyePosition = DirectX::XMVectorSet(0, 50, 200, 1);
    const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    _viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    _deviceContext->UpdateSubresource(_constantBuffers[ConstantBufferType::PerFrame].Get(), &_viewMatrix);

    static float angle = 0.0f;
    angle += 90.0f * (10.0 / 60000.0f);
    const auto rotationAxis = DirectX::XMVectorSet(0, 1, 0, 0);

    _worldMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
    _deviceContext->UpdateSubresource(_constantBuffers[ConstantBufferType::PerObject].Get(), &_worldMatrix);
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

    _deviceContext->Clear(
        _renderTarget.Get(),
        clearColor);

    _deviceContext->SetPipeline(_pipeline.get());
    _deviceContext->SetVertexBuffer(_modelVertices.Get(), 0);
    _deviceContext->SetIndexBuffer(_modelIndices.Get(), 0);
    _deviceContext->SetViewport(viewport);
    _deviceContext->DrawIndexed();
    _swapChain->Present(1, 0);
}

bool LoadingMeshesApplication::LoadModel(const std::string& filePath)
{
    constexpr uint32_t importFlags = aiProcess_Triangulate | aiProcess_FlipUVs;

    Assimp::Importer sceneImporter;
    const aiScene* scene = sceneImporter.ReadFile(filePath, importFlags);

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
