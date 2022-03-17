#pragma once

#include <d3d11.h>

#include <string_view>

#include "Definitions.hpp"

class ShaderFactory
{
public:
    ShaderFactory(const WRL::ComPtr<ID3D11Device>& device);

    [[nodiscard]] WRL::ComPtr<ID3D11VertexShader> CreateVertexShader(
        const std::wstring_view fileName,
        WRL::ComPtr<ID3DBlob>& vertexShaderBlob) const;
    [[nodiscard]] WRL::ComPtr<ID3D11PixelShader> CreatePixelShader(std::wstring_view fileName) const;

private:
    bool CompileShader(
        const std::wstring_view fileName,
        const std::string_view entryPoint,
        const std::string_view profile,
        WRL::ComPtr<ID3DBlob>& shaderBlob) const;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
};
