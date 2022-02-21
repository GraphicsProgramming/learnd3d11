#include "HelloTriangle.hpp"

int main(int argc, char* argv[])
{
    HelloTriangleApplication app{"LearnD3D11 - Hello Triangle"};
    if (!app.Initialize())
    {
        return -1;
    }
    app.Run();
}
