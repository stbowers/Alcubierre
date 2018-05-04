# Alcubierre
A FTL-like video game programmed in C using the ncurses library

Programmed for CS 2060, section 3, Spring 2018

University of Colorado at Colorado Springs

Licensed under the MIT License (LICENSE.txt)

## Building
Use cmake to generate build files for your individual system. Works with Linux, macOS, and Windows

The game looks for an assets directory in the working path to load textures frome, so either run the executable from
the root of this repo, or copy the assets folder to wherever the game is running from. If any assets aren't found you'll
probably get a cryptic zlib error and segfault, since proper error handling wasn't implemented due to time constraints.

Linux:
```
> mkdir bin
> cd bin
> cmake ..
> make
```

macOS:
```
> mkdir bin
> cd bin
> cmake ..

Open the generated project file in XCode and compile
```

Windows:
```
> mkdir bin
> cd bin
> cmake ..

Open the generated project file in Visual Studio and compile
```
