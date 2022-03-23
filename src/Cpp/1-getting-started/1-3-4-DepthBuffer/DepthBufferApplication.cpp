#include "DepthBufferApplication.hpp"
#include "DeviceContext.hpp"
#include "PipelineFactory.hpp"
#include "Pipeline.hpp"
#include "TextureFactory.hpp"
#include "ModelFactory.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3dcompiler.h>

#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_dx11.h>
#include <imgui/backend/imgui_impl_glfw.h>

#include <iostream>

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

DepthBufferApplication::DepthBufferApplication(const std::string_view title)
    : Application(title)
{
}

DepthBufferApplication::~DepthBufferApplication()
{
    _deviceContext->Flush();
    _depthDisabledDepthStencilState.Reset();
    _depthEnabledLessDepthStencilState.Reset();
    _depthEnabledLessEqualDepthStencilState.Reset();
    _depthEnabledAlwaysDepthStencilState.Reset();
    _depthEnabledNeverDepthStencilState.Reset();
    _depthEnabledEqualDepthStencilState.Reset();
    _depthEnabledNotEqualDepthStencilState.Reset();
    _depthEnabledGreaterDepthStencilState.Reset();
    _depthEnabledGreaterEqualDepthStencilState.Reset();

    _depthStencilView.Reset();
    _constantBuffers[ConstantBufferType::PerApplication].Reset();
    _constantBuffers[ConstantBufferType::PerFrame].Reset();
    _constantBuffers[ConstantBufferType::PerObject].Reset();
    _textureSrv.Reset();
    _pipeline.reset();
    _pipelineFactory.reset();
    _modelVertices.Reset();
    _modelIndices.Reset();
    _modelFactory.reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(_imGuiContext);

    Application::Cleanup();
}

bool DepthBufferApplication::Initialize()
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
    uint32_t deviceFlags = 0;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

    WRL::ComPtr<ID3D11DeviceContext> deviceContext = nullptr;
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

    InitializeImGui();

    constexpr char deviceName[] = "DEV_Main";
    _device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(deviceContext.Get(), "CTX_Main");

    _deviceContext = std::make_unique<DeviceContext>(_device, std::move(deviceContext));

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

    _pipelineFactory = std::make_unique<PipelineFactory>(_device);
    _textureFactory = std::make_unique<TextureFactory>(_device);
    _modelFactory = std::make_unique<ModelFactory>(_device);

    return true;
}

bool DepthBufferApplication::Load()
{
    PipelineDescriptor pipelineDescriptor = {};
    pipelineDescriptor.VertexFilePath = L"Assets/Shaders/Main.vs.hlsl";
    pipelineDescriptor.PixelFilePath = L"Assets/Shaders/Main.ps.hlsl";
    pipelineDescriptor.VertexType = VertexType::PositionColorUv;
    if (!_pipelineFactory->CreatePipeline(pipelineDescriptor, _pipeline))
    {
        std::cout << "PipelineFactory: Unable to create pipeline\n";
        return false;
    }

    _pipeline->SetViewport(0.0f, 0.0f, GetWindowWidth(), GetWindowHeight());

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

    if (!_modelFactory->LoadModel(
        "Assets/Models/SM_Good_Froge.fbx",
        _modelVertices,
        &_modelVertexCount,
        _modelIndices,
        &_modelIndexCount))
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

    _pipeline->BindVertexStageConstantBuffer(0, _constantBuffers[ConstantBufferType::PerApplication].Get());
    _pipeline->BindVertexStageConstantBuffer(1, _constantBuffers[ConstantBufferType::PerFrame].Get());
    _pipeline->BindVertexStageConstantBuffer(2, _constantBuffers[ConstantBufferType::PerObject].Get());

    _projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        GetWindowWidth() / static_cast<float>(GetWindowHeight()),
        0.1f,
        512.0f);
    _deviceContext->UpdateSubresource(_constantBuffers[PerApplication].Get(), &_projectionMatrix);

    if (!CreateDepthStencilStates())
    {
        return false;
    }

    return true;
}

bool DepthBufferApplication::CreateSwapchainResources()
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

    WRL::ComPtr<ID3D11Texture2D> depthBuffer = nullptr;

    D3D11_TEXTURE2D_DESC depthStencilBufferDescriptor = {};
    depthStencilBufferDescriptor.ArraySize = 1;
    depthStencilBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDescriptor.CPUAccessFlags = 0;
    depthStencilBufferDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDescriptor.Width = GetWindowWidth();
    depthStencilBufferDescriptor.Height = GetWindowHeight();
    depthStencilBufferDescriptor.MipLevels = 1;
    depthStencilBufferDescriptor.SampleDesc.Count = 1;
    depthStencilBufferDescriptor.SampleDesc.Quality = 0;
    depthStencilBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    if (FAILED(_device->CreateTexture2D(
        &depthStencilBufferDescriptor,
        nullptr,
        &depthBuffer)))
    {
        std::cout << "D3D11: Failed to create depth buffer\n";
        return false;
    }

    if (FAILED(_device->CreateDepthStencilView(
        depthBuffer.Get(),
        nullptr,
        &_depthStencilView)))
    {
        std::cout << "D3D11: Failed to create SRV from Back Buffer\n";
        return false;
    }

    return true;
}

void DepthBufferApplication::DestroySwapchainResources()
{
    _depthStencilView.Reset();
    _renderTarget.Reset();
}

void DepthBufferApplication::OnResize(
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

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(GetWindowWidth(), GetWindowHeight());

    _projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        GetWindowWidth() / static_cast<float>(GetWindowHeight()),
        0.1f,
        512);
    _deviceContext->UpdateSubresource(_constantBuffers[PerApplication].Get(), &_projectionMatrix);
}

void DepthBufferApplication::Update()
{
    const auto eyePosition = DirectX::XMVectorSet(0, 50, 200, 1);
    const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    _viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    _deviceContext->UpdateSubresource(_constantBuffers[PerFrame].Get(), &_viewMatrix);

    static float angle = 0.0f;
    if (_toggledRotation)
    {
        angle += 90.0f * (10.0 / 60000.0f);
    }
    else
    {
        angle -= 90.0f * (10.0 / 60000.0f);
    }

    const auto rotationAxis = DirectX::XMVectorSet(0, 1, 0, 0);

    _worldMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
    _deviceContext->UpdateSubresource(_constantBuffers[PerObject].Get(), &_worldMatrix);
}

void DepthBufferApplication::Render()
{
    constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

    _deviceContext->Clear(
        _renderTarget.Get(),
        clearColor,
        _depthStencilView.Get(),
        1.0f);
    _deviceContext->SetPipeline(_pipeline.get());
    _deviceContext->SetVertexBuffer(_modelVertices.Get(), 0);
    _deviceContext->SetIndexBuffer(_modelIndices.Get(), 0);

    _deviceContext->DrawIndexed();

    RenderUi();
    _swapChain->Present(1, 0);
}

void DepthBufferApplication::RenderUi()
{
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Hello Froge"))
    {
        ImGui::Checkbox("Toggle Rotation", &_toggledRotation);

        ImGui::TextUnformatted("Depth State");
        ImGui::RadioButton("Disabled", &_selectedDepthFunction, 0);
        ImGui::RadioButton("Less", &_selectedDepthFunction, 1);
        ImGui::RadioButton("LessEqual", &_selectedDepthFunction, 2);
        ImGui::RadioButton("Greater", &_selectedDepthFunction, 3);
        ImGui::RadioButton("GreaterEqual", &_selectedDepthFunction, 4);
        ImGui::RadioButton("Equal", &_selectedDepthFunction, 5);
        ImGui::RadioButton("NotEqual", &_selectedDepthFunction, 6);
        ImGui::RadioButton("Always", &_selectedDepthFunction, 7);
        ImGui::RadioButton("Never", &_selectedDepthFunction, 8);

        switch (_selectedDepthFunction)
        {
        case 0: _pipeline->SetDepthStencilState(_depthDisabledDepthStencilState.Get());
            break;
        case 1: _pipeline->SetDepthStencilState(_depthEnabledLessDepthStencilState.Get());
            break;
        case 2: _pipeline->SetDepthStencilState(_depthEnabledLessEqualDepthStencilState.Get());
            break;
        case 3: _pipeline->SetDepthStencilState(_depthEnabledGreaterDepthStencilState.Get());
            break;
        case 4: _pipeline->SetDepthStencilState(_depthEnabledGreaterEqualDepthStencilState.Get());
            break;
        case 5: _pipeline->SetDepthStencilState(_depthEnabledEqualDepthStencilState.Get());
            break;
        case 6: _pipeline->SetDepthStencilState(_depthEnabledNotEqualDepthStencilState.Get());
            break;
        case 7: _pipeline->SetDepthStencilState(_depthEnabledAlwaysDepthStencilState.Get());
            break;
        case 8: _pipeline->SetDepthStencilState(_depthEnabledNeverDepthStencilState.Get());
            break;
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DepthBufferApplication::InitializeImGui()
{
    _imGuiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(GetWindowWidth(), GetWindowHeight());

    ImGui_ImplGlfw_InitForOther(GetWindow(), true);
}

bool DepthBufferApplication::CreateDepthStencilStates()
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
    depthStencilDescriptor.DepthEnable = false;
    depthStencilDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
    depthStencilDescriptor.StencilEnable = false;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthDisabledDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create disabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthEnable = true;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledLessDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledLessEqualDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledAlwaysDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledNeverDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledGreaterDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledGreaterEqualDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_EQUAL;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledEqualDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDescriptor, &_depthEnabledNotEqualDepthStencilState)))
    {
        std::cout << "D3D11: Unable to create enabled depth stencil state\n";
        return false;
    }

    return true;
}
