#include "Pipeline.hpp"

void Pipeline::SetViewport(
    const float left,
    const float top,
    const float width,
    const float height)
{
    _viewport.TopLeftX = left;
    _viewport.TopLeftY = top;
    _viewport.Width = width;
    _viewport.Height = height;
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;
}
