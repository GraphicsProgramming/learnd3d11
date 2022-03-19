#pragma once

#include <DirectXMath.h>

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;
using Uv = DirectX::XMFLOAT2;

struct VertexPositionColorUv
{
    Position position;
    Color color;
    Uv uv;
};
