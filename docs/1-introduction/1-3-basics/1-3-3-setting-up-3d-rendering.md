# Setting up 3D Rendering

Even though the API is called "Direct 3D 11" weirdly enough we can't just simply render a bunch of vertices and have it show up as we expect it to look. 
Because our screen is 2D, we need to be able to "transform" our 3D model into a 2D space.

For this we'll need to take a light dive into "Matrix Math", whilst understanding the math behind it all can be really helpful (especially once you start doing more advanced stuff), We'll use a library for all of this and only stick to top-level concepts as not to make this tutorial a math-lesson.

The "Transformation" we're concerned with is composed out of a set of multiple matrices:

For our 3D object we have a:

- Rotation matrix (contains the rotation)
- Scaling matrix (contains the scale)
- Translation matrix (contains the position)

These three matrices will compose a matrix we call the "Model matrix", which we get by multiplying them together by doing:

`ModelMatrix = ((Rotation * Scaling) * Translation)`. 

This is one of the two key components to getting a 3D object on our screen.

Next up we generally have a something we could call a "Camera" containing a:

- View matrix (contains the camera position + rotation information)
- Projection matrix

The Projection matrix needs a bit more explanation, most traditional camera setups have two modi: "Perspective" and "Orthographic", this is what our "projection matrix" contains alongside, the "Field of View" or "orthographic size" and our "Near" and "Far" plane whose importance will be clear in a moment. 

Multiplying these two matrices together results in our "ViewProjection matrix" or "Camera matrix", this is the other key component for getting our 3D object on our screen, which we get by doing:

`ViewProjection = View * Projection`.

Now that we have our Model matrix and ViewProjection we can make our final matrix, the "world matrix" which we get by multiplying them together: `WorldMatrix = ModelMatrix * ViewProjection`
This matrix is what we transform every vertex with in order to get our 3D object on our 2D screen.

Because all these main matrix multiplications happen infrequently enough, we "can" do this on the CPU, we only have to recalculate the matrices of 3D objects when/if they move/scale/rotate which for most level geometry is almost never. However...

The only exception is the camera, which tends to move almost every frame, however we tend to only have 1 of them (or an insignificant amount in other cases).
The keen readers might realize that because of the the fact that we recalculate the camera matrix, we have to recalculate the world matrix for 'every' 3D object.

What we cannot do however (or well, not with high-poly objects) is transform every vertex on the CPU with the world matrix, luckily GPU's are practically built for this and thus are very good at it. 
But that means we need a way to get the matrix we need over there somehow.

In D3D11 we have a thing called "Constant Buffers", this is special buffer to contain values that the GPU can expect not to change during a draw call, this means the values are "constant" or "uniform" for the entire shader invocation.
This is a great place to put our matrix.

!!! error
	explain and create Constant Buffers
	
!!! error
    add to shader, do vertex multiplication with worldmatrix (or model \* camera) there.

!!! error
    end up with triangle in 3D space before we move on to next chapter


[Next chapter](./1-3-4-3d-rendering.md)
