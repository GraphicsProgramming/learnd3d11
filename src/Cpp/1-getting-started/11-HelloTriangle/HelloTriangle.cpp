#include "Graphics/D3DContext.hpp"
#include "HelloTriangle.hpp"
#include "Input/Input.hpp"

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>

HelloTriangleApplication::HelloTriangleApplication(const std::string_view title)
    : Application(title)
{
}

HelloTriangleApplication::~HelloTriangleApplication()
{
    _triangle.reset();
    _pipeline.reset();
    _dxContext.reset();
    Application::Cleanup();
}


bool HelloTriangleApplication::Initialize()
{
    // Uhhh this is a mess and should be cleaned up.
    if (!Application::Initialize())
    {
        return false;
    }
    _dxContext = std::make_unique<D3DContext>();
    if (!_dxContext->Initialize(*this))
    {
        std::cout << "Failed to initialize D3D\n";
        Cleanup();
        return false;
    }
    _pipeline = std::make_unique<GraphicsPipeline>();
    if (!_dxContext->MakeGraphicsPipeline(
            L"Assets/Shaders/Main.vs.hlsl",
            L"Assets/Shaders/Main.ps.hlsl",
            _pipeline.get()))
    {
        std::cout << "Failed to create graphics pipeline\n";
        Cleanup();
        return false;
    }
    std::vector vertices = {
         0.0f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
    };
    std::vector indices = {
        0u, 1u, 2u
    };
    _triangle = std::make_unique<StaticMesh>();
    if (!_dxContext->MakeStaticMesh(
            std::move(vertices),
            std::move(indices),
            _triangle.get()))
    {
        std::cout << "Failed to create triangle mesh\n";
        Cleanup();
        return false;
    }
    return true;
}


void HelloTriangleApplication::Render()
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = GetWindowWidth();
    viewport.Height   = GetWindowHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    const UINT vertexStride = sizeof(float[3]);
    const UINT vertexOffset = 0;
    ID3D11RenderTargetView* renderTarget = _dxContext->GetRenderTarget();

    _dxContext->GetDeviceContext()->OMSetRenderTargets(1, &renderTarget, nullptr);
    _dxContext->GetDeviceContext()->RSSetViewports(1, &viewport);
    _dxContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _dxContext->GetDeviceContext()->IASetInputLayout(_pipeline->inpuLayout.Get());
    _dxContext->GetDeviceContext()->IASetIndexBuffer(_triangle->indices.Get(), DXGI_FORMAT_R32_UINT, 0);
    _dxContext->GetDeviceContext()->IASetVertexBuffers(0, 1, _triangle->vertices.GetAddressOf(), &vertexStride, &vertexOffset);
    _dxContext->GetDeviceContext()->VSSetShader(_pipeline->vertexShader.Get(), nullptr, 0);
    _dxContext->GetDeviceContext()->PSSetShader(_pipeline->pixelShader.Get(), nullptr, 0);
    _dxContext->GetDeviceContext()->ClearRenderTargetView(renderTarget, clearColor);
    _dxContext->GetDeviceContext()->DrawIndexed(3, 0, 0);
    _dxContext->GetSwapChain()->Present(0, 0);
}

void HelloTriangleApplication::Update()
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        std::cout << "Key: Escape\n";
        Close();
    }

    if (IsButtonPressed(GLFW_MOUSE_BUTTON_1))
    {
        const auto cursorPosition = _input->GetMouse().CursorPosition;
        std::cout << std::to_string(cursorPosition.x) << ", " << std::to_string(cursorPosition.y) << "\n";
    }
}
