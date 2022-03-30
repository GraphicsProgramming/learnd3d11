#include "ModelFactory.hpp"
#include "VertexType.hpp"

#undef min
#undef max

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <vector>

ModelFactory::ModelFactory(const WRL::ComPtr<ID3D11Device>& device)
{
    _device = device;
}

bool ModelFactory::LoadModel(
    const std::string& filePath,
    WRL::ComPtr<ID3D11Buffer>& vertexBuffer,
    uint32_t* vertexCount,
    WRL::ComPtr<ID3D11Buffer>& indexBuffer,
    uint32_t* indexCount)
{
    constexpr uint32_t importFlags = aiProcess_Triangulate | aiProcess_FlipUVs;
    const std::string fileName{ filePath.begin(), filePath.end() };

    Assimp::Importer sceneImporter;
    const aiScene* scene = sceneImporter.ReadFile(fileName.c_str(), importFlags);

    if (scene == nullptr)
    {
        std::cout << "ASSIMP: Failed to load model file\n";
        return false;
    }

    if (!scene->HasMeshes())
    {
        std::cout << "ASSIMP: Model file is empty\n";
        return false;
    }

    const aiMesh* mesh = scene->mMeshes[0];
    if (!mesh->HasPositions())
    {
        std::cout << "ASSIMP: Model mesh has no positions\n";
        return false;
    }

    constexpr Color defaultColor = Color{ 0.5f, 0.5f, 0.5f };
    constexpr Uv defaultUv = Uv{ 0.0f, 0.0f };
    std::vector<VertexPositionColorUv> vertices;
    for (size_t i = 0; i < (mesh->mNumVertices); i++)
    {
        const Position& position = Position{ mesh->mVertices[i].x / 10.0f, mesh->mVertices[i].y / 10.0f, mesh->mVertices[i].z / 10.0f };
        const Color& color = mesh->HasVertexColors(0)
            ? Color{ mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b }
        : defaultColor;
        const Uv& uv = mesh->HasTextureCoords(0)
            ? Uv{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }
        : defaultUv;

        vertices.push_back(VertexPositionColorUv{ Position{position}, Color{color}, Uv{uv} });
    }

    *vertexCount = static_cast<uint32_t>(vertices.size());

    D3D11_BUFFER_DESC vertexBufferDescriptor = {};
    vertexBufferDescriptor.ByteWidth = static_cast<uint32_t>(sizeof(VertexPositionColorUv) * vertices.size());
    vertexBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    vertexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexBufferData = {};
    vertexBufferData.pSysMem = vertices.data();

    if (FAILED(_device->CreateBuffer(
        &vertexBufferDescriptor,
        &vertexBufferData,
        &vertexBuffer)))
    {
        std::cout << "D3D11: Failed to create model vertex buffer\n";
        return false;
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        for (uint32_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    *indexCount = static_cast<uint32_t>(indices.size());

    D3D11_BUFFER_DESC indexBufferDescriptor = {};
    indexBufferDescriptor.ByteWidth = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());
    indexBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    indexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexBufferData = {};
    indexBufferData.pSysMem = indices.data();

    if (FAILED(_device->CreateBuffer(
        &indexBufferDescriptor,
        &indexBufferData,
        &indexBuffer)))
    {
        std::cout << "D3D11: Failed to create model index buffer\n";
        return false;
    }

    return true;
}
