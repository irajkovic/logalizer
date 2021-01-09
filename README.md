Logalizer
=========

## Introduction

Logalizer is a log analysis tool. It can read from multiple inputs and classify read lines against regular expressions.
Each line is marked with a color representing its filter, allowing for a quick visual identification. Additionally, line
visibility can be enabled/disabled to quickly ignore uninteresting lines and reduce the clutter.

The UI is implemented with ncurses, with a goal of easy integration into existing console-based development workflows.

## Dependencies

The program depends on ncurses and uses cmake to build. On Debian based distributions, these can be installed with:

    sudo apt install cmake libncurses5-dev

Alternatively, the Docker environment can also be used to build in a platform independent way.

## Building 

### Optional: Prepare the Docker environment

First build the docker image with:

    docker build -t ubuntu-log .

Now run the container with:

    docker run -it -v `pwd`:`pwd` -w `pwd` ubuntu-log /bin/bash

### Compiling the program

To build the program, run the following:

    mkdir build && cd build
    cmake ..
    make

## Running

From the build folder, run the program with:

    ./log-analyzer <options>
