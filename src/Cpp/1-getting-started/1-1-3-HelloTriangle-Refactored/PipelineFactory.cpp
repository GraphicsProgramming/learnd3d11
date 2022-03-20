#include "PipelineFactory.hpp"
#include "Pipeline.hpp"

#include <d3dcompiler.h>
#include <wrl.h>

#include <iostream>

static size_t GetLayoutByteSize(VertexType vertexType)
{
    switch (vertexType)
    {
        case VertexType::PositionColor: return sizeof(VertexPositionColor);
    }
    return 0;
}

PipelineFactory::PipelineFactory(const WRL::ComPtr<ID3D11Device>& device)
{
    _device = device;

    _layoutMap[VertexType::PositionColor] =
    {
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                offsetof(VertexPositionColor, position),
                D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR",
                0,
                DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                offsetof(VertexPositionColor, color),
                D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
        }
    };
}

bool PipelineFactory::CreatePipeline(
    const PipelineSettings& settings,
    std::unique_ptr<Pipeline>& pipeline)
{
    WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    pipeline = std::make_unique<Pipeline>();
    pipeline->_vertexShader = CreateVertexShader(settings.VertexFilePath, vertexShaderBlob);
    pipeline->_pixelShader = CreatePixelShader(settings.PixelFilePath);
    if (!CreateInputLayout(settings.VertexType, vertexShaderBlob, pipeline->_inputLayout))
    {
        return false;
    }
    pipeline->_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pipeline->_vertexSize = GetLayoutByteSize(settings.VertexType);
    return true;
}

bool PipelineFactory::CompileShader(
    const std::wstring& fileName,
    const std::string& entryPoint,
    const std::string& profile,
    WRL::ComPtr<ID3DBlob>& shaderBlob) const
{
    constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    WRL::ComPtr<ID3DBlob> tempShaderBlob = nullptr;
    WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    if (FAILED(D3DCompileFromFile(
        fileName.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.data(),
        profile.data(),
        compileFlags,
        0,
        &tempShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Failed to read shader from file\n";
        if (errorBlob != nullptr)
        {
            std::cout << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        }

        return false;
    }

    shaderBlob = std::move(tempShaderBlob);
    return true;
}

WRL::ComPtr<ID3D11VertexShader> PipelineFactory::CreateVertexShader(
    const std::wstring& fileName,
    WRL::ComPtr<ID3DBlob>& vertexShaderBlob) const
{
    if (!CompileShader(fileName, "Main", "vs_5_0", vertexShaderBlob))
    {
        return nullptr;
    }

    WRL::ComPtr<ID3D11VertexShader> vertexShader;
    if (FAILED(_device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        std::cout << "D3D11: Failed to compile vertex shader\n";
        return nullptr;
    }

    return vertexShader;
}

WRL::ComPtr<ID3D11PixelShader> PipelineFactory::CreatePixelShader(const std::wstring& fileName) const
{
    WRL::ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
    if (!CompileShader(fileName, "Main", "ps_5_0", pixelShaderBlob))
    {
        return nullptr;
    }

    WRL::ComPtr<ID3D11PixelShader> pixelShader;
    if (FAILED(_device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader)))
    {
        std::cout << "D3D11: Failed to compile pixel shader\n";
        return nullptr;
    }

    return pixelShader;
}

bool PipelineFactory::CreateInputLayout(
    const VertexType layoutInfo,
    const WRL::ComPtr<ID3DBlob>& vertexBlob,
    WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
    const std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = _layoutMap[layoutInfo];
    if (FAILED(_device->CreateInputLayout(
        inputLayoutDesc.data(),
        inputLayoutDesc.size(),
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        &inputLayout)))
    {
        std::cout << "D3D11: Failed to create the input layout";
        return false;
    }
    return true;
}
