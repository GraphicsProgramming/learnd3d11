#pragma once

#include "Definitions.hpp"

#include <d3d11.h>

class Pipeline
{
public:
    friend class PipelineFactory;
    friend class DeviceContext;

private:
    WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
    WRL::ComPtr<ID3D11InputLayout> _inputLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology = {};
    uint32_t _vertexSize = 0;
};
