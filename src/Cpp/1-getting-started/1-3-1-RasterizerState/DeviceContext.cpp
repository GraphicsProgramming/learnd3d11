#include "DeviceContext.hpp"
#include "Pipeline.hpp"

#include <utility>

DeviceContext::DeviceContext(
    const WRL::ComPtr<ID3D11Device>& device,
    WRL::ComPtr<ID3D11DeviceContext>&& deviceContext)
{
    _deviceContext = std::move(deviceContext);
    _activePipeline = nullptr;
    _drawVertices = 0;
    _drawIndices = 0;
}

DeviceContext::~DeviceContext()
{

}

void DeviceContext::Clear(
    ID3D11RenderTargetView* renderTarget,
    float clearColor[4]) const
{
    _deviceContext->ClearRenderTargetView(
        renderTarget,
        clearColor);

    _deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
}

void DeviceContext::SetPipeline(const Pipeline* pipeline)
{
    _activePipeline = pipeline;
    _deviceContext->IASetInputLayout(pipeline->_inputLayout.Get());
    _deviceContext->IASetPrimitiveTopology(pipeline->_primitiveTopology);
    _deviceContext->VSSetShader(pipeline->_vertexShader.Get(), nullptr, 0);
    _deviceContext->PSSetShader(pipeline->_pixelShader.Get(), nullptr, 0);

    _deviceContext->RSSetViewports(1, &pipeline->_viewport);
    _deviceContext->RSSetState(pipeline->_rasterizerState.Get());
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
