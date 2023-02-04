#include "LoadingMeshesApplication.hpp"
#include "DeviceContext.hpp"
#include "Pipeline.hpp"
#include "PipelineFactory.hpp"
#include "TextureFactory.hpp"

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

LoadingMeshesApplication::LoadingMeshesApplication(const std::string& title)
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
    _deviceContext.reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
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
        std::cerr << "DXGI: Failed to create factory\n";
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
        std::cerr << "D3D11: Failed to create device and device context\n";
        return false;
    }

    if (FAILED(_device.As(&_debug)))
    {
        std::cerr << "D3D11: Failed to get the debug layer from the device\n";
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
        std::cerr << "DXGI: Failed to create swapchain\n";
        return false;
    }

    CreateSwapchainResources();

    _pipelineFactory = std::make_unique<PipelineFactory>(_device);
    _textureFactory = std::make_unique<TextureFactory>(_device);

    return true;
}

bool LoadingMeshesApplication::Load()
{

    PipelineDescriptor pipelineDescriptor = {};
    pipelineDescriptor.VertexFilePath = L"Assets/Shaders/Main.vs.hlsl";
    pipelineDescriptor.PixelFilePath = L"Assets/Shaders/Main.ps.hlsl";
    pipelineDescriptor.VertexType = VertexType::PositionColorUv;
    if (!_pipelineFactory->CreatePipeline(pipelineDescriptor, _pipeline))
    {
        std::cerr << "PipelineFactory: Failed to create pipeline\n";
        return false;
    }

    _pipeline->SetViewport(
        0.0f,
        0.0f,
        static_cast<float>(GetWindowWidth()),
        static_cast<float>(GetWindowHeight()));

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
        std::cerr << "D3D11: Failed to create linear sampler state\n";
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
        std::cerr << "D3D11: Failed to create constant buffer PerApplication\n";
        return false;
    }

    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerFrame])))
    {
        std::cerr << "D3D11: Failed to create constant buffer PerFrame\n";
        return false;
    }

    if (FAILED(_device->CreateBuffer(&constantBufferDescriptor, nullptr, &_constantBuffers[ConstantBufferType::PerObject])))
    {
        std::cerr << "D3D11: Failed to create constant buffer PerObject\n";
        return false;
    }

    _pipeline->BindVertexStageConstantBuffer(0, _constantBuffers[ConstantBufferType::PerApplication].Get());
    _pipeline->BindVertexStageConstantBuffer(1, _constantBuffers[ConstantBufferType::PerFrame].Get());
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
        std::cerr << "D3D11: Failed to get back buffer from swapchain\n";
        return false;
    }

    if (FAILED(_device->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            &_renderTarget)))
    {
        std::cerr << "D3D11: Failed to create rendertarget view from back buffer\n";
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
        std::cerr << "D3D11: Failed to recreate swapchain buffers\n";
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
    Application::Update();

    const auto eyePosition = DirectX::XMVectorSet(0.0f, 50.0f, 200.0f, 1.0f);
    const auto focusPoint = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    const auto upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    _viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    _deviceContext->UpdateSubresource(_constantBuffers[ConstantBufferType::PerFrame].Get(), &_viewMatrix);

    static float angle = 0.0f;
    angle += 90.0f * (10.0f / 60000.0f);
    const auto rotationAxis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    _worldMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
    _deviceContext->UpdateSubresource(_constantBuffers[ConstantBufferType::PerObject].Get(), &_worldMatrix);
}

void LoadingMeshesApplication::Render()
{
    float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

    _deviceContext->Clear(
        _renderTarget.Get(),
        clearColor);
    _deviceContext->SetPipeline(_pipeline.get());
    _deviceContext->SetVertexBuffer(_modelVertices.Get(), 0);
    _deviceContext->SetIndexBuffer(_modelIndices.Get(), 0);
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
        std::cerr << "ASSIMP: Failed to load model file\n";
        return false;
    }

    if (!scene->HasMeshes())
    {
        std::cerr << "ASSIMP: Model file is empty\n";
        return false;
    }

    const aiMesh* mesh = scene->mMeshes[0];
    if (!mesh->HasPositions())
    {
        std::cerr << "ASSIMP: Model mesh has no positions\n";
        return false;
    }

    constexpr Color defaultColor = Color{ 0.5f, 0.5f, 0.5f };
    constexpr Uv defaultUv = Uv{ 0.0f, 0.0f };
    std::vector<VertexPositionColorUv> vertices;
    for (size_t i = 0; i < (mesh->mNumVertices); i++)
    {
        const Position& position = Position{ mesh->mVertices[i].x / 100.0f, mesh->mVertices[i].y / 100.0f, mesh->mVertices[i].z / 100.0f };
        const Color& color = mesh->HasVertexColors(0)
                               ? Color{ mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b }
                               : defaultColor;
        const Uv& uv = mesh->HasTextureCoords(0)
                         ? Uv{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }
                         : defaultUv;

        vertices.push_back(VertexPositionColorUv{ Position{ position }, Color{ color }, Uv{ uv } });
    }

    _modelVertexCount = static_cast<uint32_t>(vertices.size());

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
        std::cerr << "D3D11: Failed to create model vertex buffer\n";
        return false;
    }

    std::vector<uint32_t> indices;
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    _modelIndexCount = static_cast<uint32_t>(indices.size());

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
        std::cerr << "D3D11: Failed to create model index buffer\n";
        return false;
    }

    return true;
}
