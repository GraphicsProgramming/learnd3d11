#pragma once

#include "VertexType.hpp"
#include "Definitions.hpp"
#include <d3d11_2.h>
#include <cstdint>
#include <string>
#include <unordered_map>

struct ShaderCollectionDescriptor
{
    std::wstring VertexShaderFilePath;
    std::wstring PixelShaderFilePath;
    VertexType VertexType;
};

class ShaderCollection
{
public:

    static ShaderCollection CreateShaderCollection(
        const ShaderCollectionDescriptor& settings, ID3D11Device* device);
    static UINT GetLayoutByteSize(VertexType vertexType);

    void ApplyToContext(ID3D11DeviceContext* context);
    void Destroy();

private:

    static [[nodiscard]] WRL::ComPtr<ID3D11VertexShader> CreateVertexShader(
       ID3D11Device* device,
       const std::wstring& filePath,
       WRL::ComPtr<ID3DBlob>& vertexShaderBlob);
    static [[nodiscard]] WRL::ComPtr<ID3D11PixelShader> CreatePixelShader(
        ID3D11Device* device,
        const std::wstring& filePath);

    static bool CreateInputLayout(
        ID3D11Device* device,
        const VertexType layoutInfo,
        const WRL::ComPtr<ID3DBlob>& vertexBlob,
        WRL::ComPtr<ID3D11InputLayout>& inputLayout);

    static bool CompileShader(
        const std::wstring& filePath,
        const std::string& entryPoint,
        const std::string& profile,
        WRL::ComPtr<ID3DBlob>& shaderBlob);


    WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
    WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
    WRL::ComPtr<ID3D11InputLayout> _inputLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology = {};
    uint32_t _vertexSize = 0;
    static std::unordered_map<VertexType, std::vector<D3D11_INPUT_ELEMENT_DESC>> _layoutMap;
};
