# Laptop GPUs

Some laptops come with dedicated graphics cards not just integrated ones.

Sometimes default settings let your application only use the integrated gpu, which usually is not the best to run anything but very light games. Some games and programs have a way to select the desired graphics card, you can chose which graphics card to use via the control panel of your driver.

You can also enforce the use of the dedicated graphics card per code. Thats what we are going to show here.

Simply find a spot in your program and add the following code

```cpp
extern "C"
{
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
​  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
```

This will enforce graphics cards from NVIDIA and AMD to use the dedicated graphics card, when available.

You can find more information [here for NVIDIA](http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf) and [here for AMD](http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/)

Intel might have something similar, once they release their dedicated graphics cards for laptops.
