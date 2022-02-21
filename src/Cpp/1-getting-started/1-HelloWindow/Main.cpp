#include "HelloWindow.hpp"

int main(int argc, char* argv[])
{
    HelloWindowApplication app{"LearnD3D11 - Hello Window"};
    if (!app.Initialize())
    {
        return -1;
    }
    app.Run();
}
