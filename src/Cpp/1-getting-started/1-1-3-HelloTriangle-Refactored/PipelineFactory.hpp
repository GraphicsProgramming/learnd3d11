#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>

#include <unordered_map>
#include <string_view>
#include <memory>

#include "Definitions.hpp"

class Pipeline;

enum class VertexType
{
    PositionColor
};

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;

struct VertexPositionColor
{
    Position position;
    Color color;
};

struct PipelineSettings
{
    std::wstring_view VertexFilePath;
    std::wstring_view PixelFilePath;
    VertexType VertexType;
};

class PipelineFactory
{
public:
    PipelineFactory(const WRL::ComPtr<ID3D11Device>& device);

    bool CreatePipeline(
        const PipelineSettings& settings,
        std::unique_ptr<Pipeline>& pipeline);
private:
    [[nodiscard]] WRL::ComPtr<ID3D11VertexShader> CreateVertexShader(
        const std::wstring_view fileName,
        WRL::ComPtr<ID3DBlob>& vertexShaderBlob) const;
    [[nodiscard]] WRL::ComPtr<ID3D11PixelShader> CreatePixelShader(std::wstring_view fileName) const;

    bool CreateInputLayout(
        const VertexType layoutInfo,
        const WRL::ComPtr<ID3DBlob>& vertexBlob,
        WRL::ComPtr<ID3D11InputLayout>& inputLayout);

    bool CompileShader(
        const std::wstring_view fileName,
        const std::string_view entryPoint,
        const std::string_view profile,
        WRL::ComPtr<ID3DBlob>& shaderBlob) const;

    WRL::ComPtr<ID3D11Device> _device = nullptr;
    std::unordered_map<VertexType, std::vector<D3D11_INPUT_ELEMENT_DESC>> _layoutMap;
};
