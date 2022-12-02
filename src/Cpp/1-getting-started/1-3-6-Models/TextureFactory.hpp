#pragma once

#include "Definitions.hpp"
#include <d3d11.h>

#include <string>

class TextureFactory
{
public:
    TextureFactory(const WRL::ComPtr<ID3D11Device>& device);

    bool CreateShaderResourceViewFromFile(
        const std::wstring& filePath,
        WRL::ComPtr<ID3D11ShaderResourceView>& shaderResourceView) const;

private:
    WRL::ComPtr<ID3D11Device> _device = nullptr;
};
