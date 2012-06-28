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

This program is implemented on Ubuntu 12.04. Theoretically, this program can
be compiled on a branch of operating systems. So far, it supports Windows (only 
PBO and OpenGL API mode, and the performance is about 50% of Ubuntu's.)

How to make
-----------

Install dependency packages.

    $ sudo apt-get install build-essential libglfw-dev libglew1.6-dev libopencv-dev

Make the program on linux.

    $ make all

Make the program on windows (You have to fix dependency problems by compile
GLFW 2.7.5, GLEW 1.7 and OpenCV 2.4 on your own). 

This version uses GLFW's threading routine. As threading support will be removed 
in GLFW 3.0+, so please make sure you are not using latest version of GLFW. And 
if your compiler supports C++11's threading, please remove `-DGLFWTHREAD` from 
Makefile.mk.
    
    $ make all TARGET=windows
    
How to use
----------

Start the program and type `help` for further instructions.

    # PBO mode
    $ ./sc.pbo [--size=WIDTHxHEIGHT] [--spawn]
    # OpenGL API mode
    $ ./sc.nopbo [--size=WIDTHxHEIGHT] [--spawn]
    # CPU mode
    $ ./sc.cpu [--size=WIDTHxHEIGHT] [--spawn]

Where `--size` can be used to set the window resolution and `--spawn` will ask
SC to output u001f after each query, you may use it as a SYN symbol when you want
to control SC through standard I/O interace.

Scripting
---------

Stream Compositor supports scripting in javascript, please check out 
/script/sc.js for details. It works asynchronous to SC, and all your directions 
will be cached in a queue.

For usage information, please take a look at /script/benchmark/benchmark.js.

You have to install node.js to use scripting features.

    $ sudo apt-get install nodejs
    
Benchmark
---------

There is a built-in benchmark script. It will uses your webcam, and 3 video
files of 854x480px, 1280x720px and 1920x1080px resolution. You can use any video
source you want, by editing /script/benchmark/benchmark.js.

To run the benchmark script.

    $ node benchmark [width height]
