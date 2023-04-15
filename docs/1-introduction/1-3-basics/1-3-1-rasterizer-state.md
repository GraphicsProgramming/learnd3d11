# Rasterizer State


The rasterizer state, as the name implies, controls the rasterizer.

This is a key component in rendering, despite it looking rather trivial, rendering as we normally do (advanced techniques aside) is called "Rasterized Rendering", this is how our geometry gets plotted to our pixels on screen, how a triangle is handled and filled and how data gets passed to other shader stages.

Let's step into the few variables we can control in D3D11.

The first one is the FillMode, this only has two options and simply tells the rasterizer whether to completely fill a triangle, or to only show its edges (or as is more commonly known as "to display a wireframe"). 
That's right, we don't need a fancy shader or render the geometry in lines in order to display a wireframe, it's a built-in feature of the rasterizer, one mostly used for debugging or special kinds of visualisation.

![](../../images/1-3-1-fillmode.png)

The CullMode is a bit more useful, this controls when we cull certain triangles, and has three options, `Front`, `Back` and `None`. For most purposes we generally set this to `Back`, so what does this actually do?

Depending on our vertex winding order (which we can control in the next discussed flag), it knows what side of the triangle is the "front side", the rasterizer can automatically ignore triangles depending on whether they are facing us or not (or just simply render everything).
As was said before, generally we're not concerned with triangles that are facing away from us, as we normally can't see them anyways (you can't see the back-side of a sphere, so why render it?). However it does have its uses to do otherwise.

Think of foliage or tree leaves for example, it's way more helpful to only place a single plane of geometry and render it from both sides than to duplicate all the geometry resulting in the GPU having to do a lot more work.

Some shadowing techniques may also rely on Frontface culling in order to get better results, but we won't go into detail about that here now. 

Simple to explain, but perhaps a bit harder to understands is the "Vertex Winding Order" which is controlled by our input geometry as well as the rasteriser state.

Shortly explained, if `FrontCounterClockwise` is `true`, then a triangle is front facing if the vertices are in a **counter**-clockwise order , otherwise it is back facing.

The following vertex order makes up a "clockwise" triangle, so if the `FrontCounterClockwise` is true, this means this triangle is facing away from us, and if `CullMode` is `Back` this means we skip this entire triangle.
```
     0
    /\
   /  \
  /    \
 /______\ 
2        1 
```
For completeness-sake, the following is a "counter-clockwise triangle".
```
     0
    /\
   /  \
  /    \
 /______\ 
1        2 
```

How it looks to have our triangle in `Front` and `Back` cull-modes.
![](../../images/1-3-1-cullmode.png)

(Yes, the "missing" triangle is the expected result here, it's being culled after all!)


The last few variables we can control require some knowledge on topics we'll cover in later chapters, for now the only important one that is set to `true` by default is:
`DepthClipEnable`, which allows the rasterizer to discard triangles, or more correctly "fragments" that fall beyond our depth-range from the viewport.

We'll go into more detail for the rest once it becomes relevant, as well as the various depth-related variables.

If curious none-the-less, feel free to read up on it with the official documentation at: [D3D11_RASTERIZER_DESC](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc)

[Next chapter](./1-3-2-texturing.md) 