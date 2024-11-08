This project was the culmination of several months of me trying to learn C++ (which progressively turned into me learning C) alongside OpenGL and libraries like FreeType. I wouldn't consider this framework to be complete mainly due to the lack of audio support; the audio system that I wrote in an old version of this framework - written in object-oriented C++ using OpenAL Soft - still has not been ported over, and as I move on to other projects this likely won't ever happen.

In parts of this project I was experimenting with a coding approach inspired by [John Carmack's email in 2007 about inlined code](http://number-none.com/blow/blog/programming/2014/09/26/carmack-on-inlined-code.html), in which he recommends inlining functions (i.e. actually embedding code that would normally be segregated into another function into where the function would be called) in cases where it wouldn't result in code duplication. The core benefit of this is that it makes the control flow of your program far more transparent, as rather than having deep nests of function calls where program state could be modified in potentially unexpected ways, everything that your program is doing is plainly laid out to you in a sequential manner. The zfw_run_game() function in "zfw_game.c" is very much an attempt at this - one gargantuan function (for a project of this small scale, at least) encompassing the whole game initialisation process, restart loop (basically a full cycle of the game minus low-level initialisation work), and the main loop (the actual game loop). In retrospect, although I think the initialisation code works well in this style, the restart loop and inner main loop could definitely be refactored in some way, as in its current form it appears to me like a big bundle of mutating state that's difficult to keep in your head.

An example of a simple game made in this framework can be found [here](https://github.com/harrykwhite/zfw_example_game).