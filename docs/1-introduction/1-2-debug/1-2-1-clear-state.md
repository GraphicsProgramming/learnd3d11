# Clear State

The Clear State is useful to make sure no state from previous drawcalls is still being used.

For example:

Say we have just drawn our scene, all models and such are being drawn properly, but now we decide to add some UI, we adjust some state that normally is set-once and forget (at initialisation) like scissor rects or viewports.
But now after we're done adding our UI element draw, suddenly our models are drawing in some weird square on the screen, or models that perhaps did not have a normal-map suddenly use a UI texture as if it were one.

This is something that can happen if we don't have a clear state, and really only goes wrong if one did not think they'd had to set some part of the pipeline during some draws.

The usefulness of a Clear State generally comes in being able to enable or disable it at will and it being used as a mechanic to spot rendering bugs caused by state that was not (re)set correctly.
So as soon as weird things start to happen, you can use a clear state between say every draw, and if the bug then disappears you'll know it's very likely related to state set from a previous draw.

One can accomplish this in a few ways:

Setting their states/buffers/resources/targets to NULL where possible by hand.

Or call a more "reset it all" function that exists on the ID3D11DeviceContext aptly called __ClearState()__.
!!! info
    https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-clearstate
	
Though keep in mind that using the latter function also resets stuff like the inputlayout, primitive topology and literally everything. This might require a bit more work in making sure all rendering state is setup correctly again afterwards

In pure performance terms it can be quite wasteful to reset a whole bunch of state (or re-set it) every draw/pass/frame which is why this is viewed as a debugging option. 

In the end, one generally should make sure their draws always set (or have set) their required state so they do not need to use a clear-state.