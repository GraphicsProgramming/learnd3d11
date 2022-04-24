#include "DeviceContext.hpp"
#include "Pipeline.hpp"

#include <imgui/backend/imgui_impl_dx11.h>
#include <utility>

DeviceContext::DeviceContext(
    const WRL::ComPtr<ID3D11Device>& device,
    WRL::ComPtr<ID3D11DeviceContext>&& deviceContext)
{
    _deviceContext = std::move(deviceContext);
    _activePipeline = nullptr;
    _drawVertices = 0;
    _drawIndices = 0;
    ImGui_ImplDX11_Init(device.Get(), _deviceContext.Get());
}

DeviceContext::~DeviceContext()
{
    ImGui_ImplDX11_Shutdown();
}

void DeviceContext::Clear(
    ID3D11RenderTargetView* renderTarget,
    float clearColor[4],
    ID3D11DepthStencilView* depthStencilView,
    float clearDepth) const
{
    _deviceContext->ClearRenderTargetView(
        renderTarget,
        clearColor);
    if (depthStencilView != nullptr)
    {
        _deviceContext->ClearDepthStencilView(
            depthStencilView,
            D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH,
            clearDepth,
            0);
    }
    _deviceContext->OMSetRenderTargets(1, &renderTarget, depthStencilView);
}

void DeviceContext::SetPipeline(const Pipeline* pipeline)
{
    _activePipeline = pipeline;
    _deviceContext->IASetInputLayout(pipeline->_inputLayout.Get());
    _deviceContext->IASetPrimitiveTopology(pipeline->_primitiveTopology);
    _deviceContext->VSSetShader(pipeline->_vertexShader.Get(), nullptr, 0);
    _deviceContext->PSSetShader(pipeline->_pixelShader.Get(), nullptr, 0);

    for (auto [descriptor, resource] : pipeline->_resources)
    {
        switch (descriptor.Type)
        {
            case ResourceType::Sampler:
                _deviceContext->PSSetSamplers(descriptor.SlotIndex, 1, reinterpret_cast<ID3D11SamplerState**>(&resource));
                break;

            case ResourceType::Texture:
                _deviceContext->PSSetShaderResources(descriptor.SlotIndex, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&resource));
                break;

            case ResourceType::Buffer:
                switch (descriptor.Stage)
                {
                    case ResourceStage::VertexStage:
                        _deviceContext->VSSetConstantBuffers(descriptor.SlotIndex, 1, reinterpret_cast<ID3D11Buffer**>(&resource));
                        break;
                    case ResourceStage::PixelStage:
                        _deviceContext->PSSetConstantBuffers(descriptor.SlotIndex, 1, reinterpret_cast<ID3D11Buffer**>(&resource));
                        break;
                }
                break;
        }
    }

    _deviceContext->OMSetDepthStencilState(pipeline->_depthStencilState.Get(), 0);
    _deviceContext->RSSetViewports(1, &pipeline->_viewport);
}

void DeviceContext::SetVertexBuffer(
    ID3D11Buffer* vertexBuffer,
    uint32_t vertexOffset)
{
    D3D11_BUFFER_DESC description = {};
    vertexBuffer->GetDesc(&description);
    _deviceContext->IASetVertexBuffers(
        0,
        1,
        &vertexBuffer,
        &_activePipeline->_vertexSize,
        &vertexOffset);
    _drawVertices = description.ByteWidth / _activePipeline->_vertexSize;
}

void DeviceContext::SetIndexBuffer(ID3D11Buffer* indexBuffer, uint32_t indexOffset)
{
    D3D11_BUFFER_DESC description = {};
    indexBuffer->GetDesc(&description);
    _deviceContext->IASetIndexBuffer(
        indexBuffer,
        DXGI_FORMAT::DXGI_FORMAT_R32_UINT,
        indexOffset);
    _drawIndices = description.ByteWidth / sizeof(uint32_t);
}

void DeviceContext::UpdateSubresource(ID3D11Buffer* buffer, const void* data) const
{
    _deviceContext->UpdateSubresource(
        buffer,
        0,
        nullptr,
        data,
        0,
        0);
}

void DeviceContext::Draw() const
{
    _deviceContext->Draw(_drawVertices, 0);
}

void DeviceContext::DrawIndexed() const
{
    _deviceContext->DrawIndexed(_drawIndices, 0, 0);
}

void DeviceContext::Flush() const
{
    _deviceContext->Flush();
}
