Stream Compositor
=================

Stream Compositor is a program can be used to composite multiple video streams
from file or camera source.

This program can be used to apply different transforms to each video stream,
and use these transforms to form a virtual 3D space where videos playing on
3D canvases.

If the video streams were shoot by a few camera located at the same location,
these videos can be used to make a panorama which you may look around, rotate,
and zoom in/out. (Relative theses can be found at http://wearcam.org/orbits/)

This program can be used in linux, while this program cannot be compiled
in other operating systems so far (there is no C++11 compiler which supports
both thread and chrono on Windows or OS X.)

How to make
-----------

Install dependency packages.

    $ sudo apt-get install build-essential libglfw-dev libglew1.6-dev libopencv-dev

Make the program.

    $ make all

Make on windoes.
    
    $ make all TARGET=windows
    
How to use
----------

Start the program and type `help` for further instructions.

    $ ./main

