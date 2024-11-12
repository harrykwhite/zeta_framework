<h2>About</h2>

This project was the culmination of several months of me trying to learn C++ (which progressively turned into me learning C) alongside OpenGL and libraries like FreeType. It has been written both as a learning exercise and for personal use as a simple C substitute for XNA / MonoGame (which uses C#, a language I have grown to dislike). I currently wouldn't consider ZFW to be complete mainly due to the lack of audio support; the audio system that I wrote in an old version of this framework - written in object-oriented C++ using OpenAL Soft - still has not been ported over, but I am planning on doing this within the next few months.

In parts of this project I was experimenting with a coding approach inspired by [John Carmack's email in 2007 about inlined code](http://number-none.com/blow/blog/programming/2014/09/26/carmack-on-inlined-code.html), in which he recommends manually inlining functions (i.e. actually embedding code that would normally be separated into another function into where the function would be called) in cases where it wouldn't result in code duplication. The core benefit of this is that it makes the control flow of your program far more transparent, as rather than having deep nests of function calls where program state could be modified in potentially unexpected ways, everything that your program is doing is plainly laid out to you in a sequential manner. The zfw_run_game() function in "zfw_game.c" is very much an attempt at this - one large function (for a project of this small scale, at least) encompassing the whole game initialisation process and main loop. Although I think it works decently, I do equivalently think there is value to compartmentalising certain operations into their own functions, ensuring that these functions aren't too heavily predicated on the program state being in a particular way; I might emphasise this approach more in future refactorings of the code.

A template for making games in this framework can be found [here](https://github.com/harrykwhite/zfw_game_template).

<br>

<h2>Components</h2>

**zfw:** This compiles to the main static library to be used.

**zfw_asset_packer:** This compiles to an executable which processes asset files (listed in a JSON file) and stores their essential information inside of a single "assets.zfwdat" file, which can then read by ZFW.

**zfw_common:** This compiles to a static library used by both zfw and zfw_asset_packer, and consists mostly of utility functions and structs.

<br>

<h2>Dependencies</h2>

**GLFW**: Used for windowing, input, and an OpenGL context.

**Glad**: Used for OpenGL functions.

**cJSON**: Used by the asset packer to process the packing instructions JSON file.

**FreeType**: Used by the asset packer for interpreting font files.

**stb_image**: Used by the asset packer for loading image files.

<br>
