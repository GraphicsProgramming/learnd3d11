# Creating a Window

Creating a window using the Windows API library (a.k.a WINAPI32) can be made by following these steps:"

## 1.) Fill a window class structure (WNDCLASS)

```cpp
WNDCLASS wndClass = {};
wndClass.hInstance = hInstance; //hInstance from wWinMain/WinMain
wndClass.lpfnWndProc = DefWindowProc; //function used to handle messages. We're currently using the default window message handling procedure. We will create our message handler function later...
wndClass.lpszClassName = TEXT("DirectXApplication_Tutorial"); //The TEXT() macro function is used to assure the projects compiles if UNICODE support is either enabled or disabled.
```

To note that a custom cursor, application icons and flags can be set in this structure.

## 2.) Register the window class

We register a window class using the RegisterClass function.

```cpp
if(RegisterClass(&wndClass) == 0) throw std::exception("Failed to register the window class!");; //registers the window class. A window class defines how the button behaves, i.e a button, or a label... We throw an exception if RegisterClass somehow fails...
```

Is it most of the time safe to directly call RegisterClass, without checking if the function failed.

## 3.) Finally, create the window.

Functions are created using the CreateWindowEx or CreateWindow functions. CreateWindowEx has an extra parameter, namely the first one, that allows custom flags. For example you can create a window that allows dropping a file.

```cpp
HWND MainWindow = CreateWindowEx(0, TEXT("DirectXApplication_Tutorial"), TEXT("Hello DirectX!"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 800, 600, nullptr, nullptr, hInstance, 0);
if(MainWindow == nullptr) throw std::exception("Failed to create the window!");
```

## 4.) It is required to write the message cycle to keep our application running:

This cycle keeps our application "alive". It handles the messages that are sent to our window.

`TranslateMessage()` converts virtual keys messages to character input messages. `DispatchMessage()` handles the message itself by calling `wndClass.lpfnWndProc` and by removing the message from the queue (in some cases, new messages can be sent.)

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

## 5.) You have created a window!

This is the first step in creating a native DirectX application. In the next tutorial, you'll write the message handler function (`wndClass.lpfnWndProc`).
