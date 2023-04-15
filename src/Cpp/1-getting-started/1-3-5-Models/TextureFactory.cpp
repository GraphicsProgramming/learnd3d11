#include "TextureFactory.hpp"

#include <DirectXTex.h>

#include <iostream>


#if defined(_DEBUG)
#pragma comment(lib, "FreeImageLibd.lib")
#else
#pragma comment(lib, "FreeImageLib.lib")
#endif

TextureFactory::TextureFactory(const WRL::ComPtr<ID3D11Device>& device)
{
    _device = device;
}

bool TextureFactory::CreateShaderResourceViewFromFile(
    const std::wstring& filePath,
    WRL::ComPtr<ID3D11ShaderResourceView>& shaderResourceView) const
{
    DirectX::TexMetadata metaData = {};
    DirectX::ScratchImage scratchImage;
    if (FAILED(DirectX::LoadFromDDSFile(filePath.data(), DirectX::DDS_FLAGS_NONE, &metaData, scratchImage)))
    {
        std::cerr << "DXTEX: Failed to load image\n";
        return false;
    }

    WRL::ComPtr<ID3D11Resource> texture = nullptr;
    if (FAILED(DirectX::CreateTexture(
            _device.Get(),
            scratchImage.GetImages(),
            scratchImage.GetImageCount(),
            metaData,
            &texture)))
    {
        std::cerr << "DXTEX: Failed to create texture out of image\n";
        scratchImage.Release();
        return false;
    }

    if (FAILED(DirectX::CreateShaderResourceView(
            _device.Get(),
            scratchImage.GetImages(),
            scratchImage.GetImageCount(),
            metaData,
            &shaderResourceView)))
    {
        std::cerr << "DXTEX: Failed to create shader resource view out of texture\n";
        scratchImage.Release();
        return false;
    }

    return true;
}
