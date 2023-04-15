# Models

!!! error
	Please note that this chapter is unfinished and will be finished up at some point in the future.


After being able to load and display an 3D cube the next step is getting some more exciting stuff on screen. 
Models are the next step!

Just as with textures, there's no real support for what we call "Models" in D3D11 (or any other API), with the sole exception being D3D9 and their "x" format, which we can consider antique and very much deprecated.

So we have to load models ourselves.
But before we start we need to know what a model actually is. You might open a game and say "this character is a model" or "this house is a model", but that doesn't answer much as things can easily be a lot more complicated.

Take this with a grain of salt because how a "model" is defined isn't concrete.
But a model is generally composed out of a few things, each of them optional, some of them might exist in one format and not in others.


|            |       |
|------------|-------|
| Meshes     | A collection of vertices and optionally indices|
| Materials  | Some arbitrary collection of things that tell us what a mesh looks like|
| Bones      | A collection of "bones" which can be a collection of vertices with influence amounts|
| Animations | Generally a collection of matrices for each set of bones|
| ???		| Other things, anything! Other models, Scripts, Camera's, Lights, Plumbusses, You name it! |

Now you might think: "If everything is optional, and models could have anything, how do we deal with this?"

The answer is simple: We don't!

There is no software in existance that can load and/or display all model formats and any feature they can support.
Generally only the features 'most' people care about are supported, which is generally meshes and materials, and often enough animations as well.

So how are we going to load these vague and arbitrary "models"?

Well just as with textures, we're going to use a library for it. A common choice for this is [**Assimp**](https://assimp-docs.readthedocs.io/en/latest/about/introduction.html), it is not the fastest, but it is one of the more flexible ones.
 

!!! error

    TODO: model loading


[Project on GitHub](https://github.com/GraphicsProgramming/learnd3d11/tree/main/src/Cpp/1-getting-started/1-3-2-LoadingMeshes)

[Next chapter](./1-3-6-dear-imgui.md)
