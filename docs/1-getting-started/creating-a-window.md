# Creating a Window

Creating a window using the Windows API library (a.k.a. WINAPI32) can be achieved through the following steps:

## 1.) Setup

- Make sure you have the Windows SDK and DirectX SDK installed. You can install these SDKs by going to Visual Studio Installer.
- Set the entry point function to `int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int)`. If you're using MSVC this can be done by setting the subsystem to `Windows`.


## 2.) Fill a window class structure (WNDCLASS)

```cpp
WNDCLASS wndClass = {};
wndClass.hInstance = hInstance;
wndClass.lpfnWndProc = DefWindowProc;
wndClass.lpszClassName = TEXT("DirectXApplication_Tutorial");
```

Explanation for the structure fields:

- `wndClass.hInstance` represents the handle to the application module. It is usually given as a argument in the `wWwinMain` entry-point function.

- `wndClass.lpfnWndProc` represents a pointer to the function used to handle messages. We're currently using the default window message handling procedure. We will create our message handler function later...

- `wndClass.lpszClassName` represents an identifier for the window class. The TEXT() macro function is used to assure the projects compiles if UNICODE support is either enabled or disabled.

Note that custom cursors, application icons, and flags for this window can optionally be set here.

## 3.) Register the window class

We register a window class using the RegisterClass function.

```cpp
if(RegisterClass(&wndClass) == 0) 
{
  MessageBox(nullptr,TEXT("Cannot register the window class!"), TEXT("Error!"), MB_ICONERROR | MB_OK);
  return -1;
} 
```

Most of the time it is safe to call RegisterClass without checking if the function failed.

## 4.) Finally, create the window.

Functions are created using the CreateWindowEx or CreateWindow functions. CreateWindowEx has an extra parameter, namely the first one, that allows custom flags. For example you can create a window that allows dropping a file.

```cpp
HWND MainWindow = CreateWindowEx(0, TEXT("DirectXApplication_Tutorial"), TEXT("Hello DirectX!"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 800, 600, nullptr, nullptr, hInstance, 0);
if(MainWindow == nullptr)
{
  MessageBox(nullptr, TEXT("Cannot create the window!"), TEXT("Error"), MB_ICONERROR | MB_OK);
  return -2;
}
```

## 5.) It is required to write the message loop to keep our application running:

This loop keeps our application "alive". It handles the messages that are sent to our window.

`TranslateMessage()` converts virtual keys messages to character input messages. `DispatchMessage()` handles the message itself by calling `wndClass.lpfnWndProc` and by removing the message from the queue (in some cases, new messages can be sent).

```cpp
MSG msg;
while(GetMessage(&msg,MainWindow,0,0))
{
  TranslateMessage(&msg);
  DispatchMessage(&msg);
}
```
Compiling this code with the subsystem set to windows (we must use wWinMain as the entry-point function), you'll see a window being created! It is not entirely functional, namely there's a visual glitch when resizing for example. This happends because we don't redraw the window. We don't need this in the context of a purely native DirectX application...

Therefore we won't use the `WM_PAINT` message at all. We will still need to write a message handler function to handle mouse and keyboard inputs for example...

Unfortunately, we'll have to handle the WM_QUIT and WM_CLOSE messages to terminate the application process when the window is closed. Therefore closing the window when debugging won't close the application. It should be terminated using task manager for example.

We can write a quick workaround to fix this specific issue: We check the current message in the message loop. If a quit message is being present (namely WM_CLOSE) we'll stop the loop and terminate the program execution.

```cpp
MSG msg;
while(GetMessage(&msg,MainWindow,0,0))
{
  if(msg == WM_CLOSE))
  {
    break;
  }
  TranslateMessage(&msg);
  DispatchMessage(&msg);
}
```
The `WM_CLOSE` message is sent when the window is closed, either by pressing the `X` button, by pressing `Alt+F4` or by closing the window in the taskbar.

## 6.) You have created a window!

This is the first step in creating a native DirectX application. In the next tutorial, you'll write the message handler function (`wndClass.lpfnWndProc`).

## 7.) Full source code

```cpp
#include <Windows.h>

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR cmdArgs,int nShowCmd)
{
  WNDCLASS wndClass = {};
  wndClass.hInstance = hInstance;
  wndClass.lpfnWndProc = DefWindowProc;
  wndClass.lpszClassName = TEXT("DirectXApplication_Tutorial");  
  
  if(RegisterClass(&wndClass) == 0) 
  {
    MessageBox(nullptr,TEXT("Cannot register the window class!"), TEXT("Error!"), MB_ICONERROR | MB_OK);
    return -1;
  }
  
  HWND MainWindow = CreateWindowEx(0, TEXT("DirectXApplication_Tutorial"), TEXT("Hello DirectX!"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 800, 600, nullptr, nullptr, hInstance, 0);
  if(MainWindow == nullptr)
  {
    MessageBox(nullptr, TEXT("Cannot create the window!"), TEXT("Error"), MB_ICONERROR | MB_OK);
    return -2;
  }

  MSG msg;
  while(GetMessage(&msg,MainWindow,0,0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  return 0; //Indicate to the operating system that we didn't have any errors or problems regarding exceptions, error codes, etc.
}```
