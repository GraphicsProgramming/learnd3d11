#include "ShaderFactory.hpp"

#include <d3dcompiler.h>

#include <iostream>

ShaderFactory::ShaderFactory(const WRL::ComPtr<ID3D11Device>& device)
{
    _device = device;
}

bool ShaderFactory::CompileShader(
    const std::wstring_view fileName,
    const std::string_view entryPoint,
    const std::string_view profile,
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

WRL::ComPtr<ID3D11VertexShader> ShaderFactory::CreateVertexShader(
    const std::wstring_view fileName,
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

WRL::ComPtr<ID3D11PixelShader> ShaderFactory::CreatePixelShader(const std::wstring_view fileName) const
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
