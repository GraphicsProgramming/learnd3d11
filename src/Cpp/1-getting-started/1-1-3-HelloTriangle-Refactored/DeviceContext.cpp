#include "DeviceContext.hpp"

#include <d3d11.h>
#include <utility>

#include "Pipeline.hpp"

DeviceContext::DeviceContext(WRL::ComPtr<ID3D11DeviceContext>&& deviceContext)
{
    _deviceContext = std::move(deviceContext);
}

void DeviceContext::Clear(ID3D11RenderTargetView* renderTarget, const float clearColor[4])
{
    _deviceContext->ClearRenderTargetView(renderTarget, clearColor);
    _deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
}

void DeviceContext::SetPipeline(const Pipeline* pipeline)
{
    _activePipeline = pipeline;
    _deviceContext->IASetInputLayout(pipeline->_inputLayout.Get());
    _deviceContext->IASetPrimitiveTopology(pipeline->_primitiveTopology);
    _deviceContext->VSSetShader(pipeline->_vertexShader.Get(), nullptr, 0);
    _deviceContext->PSSetShader(pipeline->_pixelShader.Get(), nullptr, 0);
}

void DeviceContext::SetVertexBuffer(ID3D11Buffer* triangleVertices, uint32_t vertexOffset)
{
    D3D11_BUFFER_DESC description = {};
    triangleVertices->GetDesc(&description);
    _deviceContext->IASetVertexBuffers(0, 1, &triangleVertices, &_activePipeline->_vertexSize, &vertexOffset);
    _drawVertices = description.ByteWidth / _activePipeline->_vertexSize;
}

void DeviceContext::SetViewport(D3D11_VIEWPORT viewport)
{
    _deviceContext->RSSetViewports(1, &viewport);
}

void DeviceContext::Draw()
{
    _deviceContext->Draw(_drawVertices, 0);
}

void DeviceContext::Flush()
{
    _deviceContext->Flush();
}


