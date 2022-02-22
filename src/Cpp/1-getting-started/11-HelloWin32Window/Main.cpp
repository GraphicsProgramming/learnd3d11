#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>

LRESULT CALLBACK WindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



int main(int argc, char* argv[])
{
    const int32_t monitorWidth = GetSystemMetrics(SM_CXSCREEN);
    const int32_t monitorHeight = GetSystemMetrics(SM_CYSCREEN);
    const int32_t windowWidth = static_cast<int32_t>(monitorWidth * 0.9f);
    const int32_t windowHeight = static_cast<int32_t>(monitorHeight * 0.9f);
    const int32_t windowLeft = monitorWidth / 2 - windowWidth / 2;
    const int32_t windowTop = monitorHeight / 2 - windowHeight / 2;
    const wchar_t* windowClassName = L"SampleWindowClass";
    const wchar_t* windowTitle = L"Learn D3D11 - Hello Win32 Window";
    const HMODULE currentInstance = GetModuleHandle(nullptr); // Equivalent to the first parameter in WinMain()
    // Registers the Window class
    WNDCLASSEX windowClassInfo;
    windowClassInfo.cbSize = sizeof(WNDCLASSEX);
    windowClassInfo.style = CS_HREDRAW | CS_VREDRAW;
    windowClassInfo.lpfnWndProc = WindowCallback;
    windowClassInfo.cbClsExtra = 0;
    windowClassInfo.cbWndExtra = 0;
    windowClassInfo.hInstance = currentInstance;
    windowClassInfo.hIcon = LoadIcon(windowClassInfo.hInstance, IDI_APPLICATION);
    windowClassInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClassInfo.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClassInfo.lpszMenuName = nullptr;
    windowClassInfo.lpszClassName = windowClassName;
    windowClassInfo.hIconSm = LoadIcon(windowClassInfo.hInstance, IDI_APPLICATION);
    if (!RegisterClassEx(&windowClassInfo))
    {
        std::cout << "Failed to register window class\n";
        return -1;
    }
    // Actually creates the window and returns a handle to it
    HWND windowHandle = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        windowClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        windowLeft,
        windowTop,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        currentInstance,
        nullptr);
    if (windowHandle == nullptr)
    {
        std::cout << "Failed to create Win32 window\n";
        return -1;
    }
    ShowWindow(windowHandle, SW_NORMAL);
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
