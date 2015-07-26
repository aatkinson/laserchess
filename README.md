CS 488 (Spring 2015) - FINAL PROJECT

Notes:

- This program compiles on the glXX machines - and should run on them - though at runtime, initialization causes a seg fault in some underlying std::write/move I have been unable to debug.

*** It does, however, run fine on my machine, which I will be using for demo purposes. ***

- Compiled, run, and demo'd on my machine running the following OS: stock Ubuntu Linux 15.04 64-bit with Unity.

- Dependencies:
--- Qt 5.1.0+
--- OpenGl 3.0 Mesa 10.5.2 (OpenGL version on my machine)
--- libsdl1.2-dev
--- libsdl-mixer1.2-dev


To Build:

1. `cd handin/A5/src`
2. `qmake`
3. `make`

To Run:

1. `cd handin/A5/src`
2. `./laserchess`

