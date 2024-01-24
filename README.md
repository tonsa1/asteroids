This is my game engine. Everything has been written by me. No third party libraries used.

The demo is a simple Asteroids game.
The rendering is done by a software rasterizer that is SIMD optimized with AVX.
Ship and bullets use a line rasterizer and the asteroids use a scanline rasterizer. Scanline is not optimized yet.


The source code can be found in the "Code" directory and the executable is in "release".

You can build the project by setting up an environment with vcvarsall and then running build.bat

Arrow keys: Movement
Spacebar: Shooting
ESC: Reset

