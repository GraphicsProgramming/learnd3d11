# 3D Rendering

Coming from the previous chapter, two changes have been made, instead of a single triangle, we've added a list of vertices and indices in order to create a cube mesh, and to make things more clear, we're relying on colors instead of a texture for now.

Without any other changes to our main code, our cube will show up in 3D just as it did with our triangle, quite easy huh!

![](../../images/1-3-4-insidecube.png)

But something seems wrong, our triangle didn't look so weird, why are we looking inside the cube and seeing the other side sometimes? 

Well, we're still missing one critical part in order to get 3D rendering to work properly. 

## The Depth Buffer

In order to fix our weird looking cube we'll need a depth buffer, this is a special kind of render target that keeps track of the "depth" of each fragment on the screen, basically saying "how far away is this pixel on the screen".
This depth buffer can then be used by special hardware on the GPU to see if the fragment we're working on is behind or in front of the previous fragment.

This is called "Depth Testing", a very important concept within common rendering.

!!! error
	add depth buffer + state

!!! error
	explain rasterizer desc -> DepthClipEnable (visually as well if possible)

 
[Next chapter](./1-3-5-models.md)