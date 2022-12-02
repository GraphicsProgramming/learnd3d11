#pragma once

#include <d3d11.h>

#include "Definitions.hpp"

#include <string>

class ModelFactory
{
public:
    ModelFactory(const WRL::ComPtr<ID3D11Device>& device);

    bool LoadModel(
        const std::string& filePath,
        WRL::ComPtr<ID3D11Buffer>& vertexBuffer,
        uint32_t* vertexCount,
        WRL::ComPtr<ID3D11Buffer>& indexBuffer,
        uint32_t* indexCount);

private:
    WRL::ComPtr<ID3D11Device> _device = nullptr;
};
