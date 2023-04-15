#pragma once

#include <DirectXMath.h>

enum class VertexType
{
    PositionColor,
    PositionColorUv
};

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;
using Uv = DirectX::XMFLOAT2;

struct VertexPositionColor
{
    Position position;
    Color color;
};

struct VertexPositionColorUv
{
    Position position;
    Color color;
    Uv uv;
};
