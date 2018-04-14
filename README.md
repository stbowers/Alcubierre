Alcubierre
A roguelike programmed in C with the ncurses library.

For the final in CS 2060 - 003 (Spring 2018)

The C++ files in the code overview are from CMake, the game itself is written in 100% C

.xp and .axp Files:
This engine uses .xp files from REXPaint as its texture files, which makes creating and loading ascii art easy.
For animation the engine uses a custom format .axp (animated xp file) which includes a version number and fps count before the .xp data.
To create a .axp file:

Unpack the .xp file (.xp files are compressed with gzip)
cat file.xp | gzip -d > file.xp.raw

Insert the .axp header before the .xp file and then recompress
printf "\x01\x00\x00\x00\x0a\x00\x00\x00" | cat file.xp.raw | gzip > animated_file.axp

Where \x0a (and the remaining 3 bytes, in little endian order) is replaced with the fps for the animation (10fps for \x0a)
