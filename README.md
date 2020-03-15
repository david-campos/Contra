# Recreation of Contra
***Project for Game Engine Architecture by David Campos Rodríguez***

This project is a recreation of the famous game Contra for the
Nintendo Entertainment System I created as part of the Game Engine Architecture
course during my master's program in Game Design and Technology
in the University of Gothenburg. Time constraints limited the total number
of levels that could be done, but I **may** continue working in the project
on the future just to have it a bit more finished.

The report elaborated for the course can be found in the folder course_docs.

## Windows release
There is a release made with mingw for Windows, 32bits in release/windows32bits_release.rar.

## Compilation notes
The game uses the SDL, SDL_ttf, SDL_image, SDL_mixer and yaml-cpp libraries.
All the SDL libraries can be found in the official SDL page. Download the
adequate version for your compiler.

yaml-cpp can be downloaded from https://github.com/jbeder/yaml-cpp.
However, I include a copy of it in the *third-party* folder.

The CMakeLists.txt file is configured to use the libraries inside cmake/mingw
when compiling with mingw and to compile and use the yaml-cpp library in third-party
too. If other compiler is used, the finders in cmake will be used
to try to find the libraries in the system (in Linux it works better for me).

If you are having trouble compiling the project you may try to modify
the CMakeLists.txt file similarly to what I did for MINGW and manually
set the paths to your libraries. The finder files in cmake are taken from
arbitrary github repositories and therefore they may contain errors
for compiling in some systems.

