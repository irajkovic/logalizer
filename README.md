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

For more information, use the built in help:

	./log-analyzer -h
	
	Logalizer
	
	Log Analysis tool.
	
	Options:
	  -i <file>
	     Input file to be read.
	  -f <name:regex>
	     Defines a regular expression with the given name.
	     Matching lines will be marked with a color and
	     their visibility can be toggled.
	  -e <name:command>
	     Defines a command to be executed whenever a line
	     with matches a filter with the same name.
	     The whole line will be passed to the given command
	     as its first argument and anything outputed to the
	     standard output by the command will be recorded and
	     displayed as a comment.
	  -h Prints this help.
	
	Example:
	
	  log-analyzer -i  <(journalctl) -f 'KERNEL:.*kernel.*' 'SYSTEMD:.*systemd.*' 2> err.txt 

	     Reads the contents of journalctl and marks all lines containing "kernel" and "systemd". 

## Unit tests and code coverage

To compile the unit tests, set the `ENABLE_TESTS` CMake flag. To enable the code coverage, set the
`ENABLE_COVERAGE` flag:
    
    cmake -DENABLE_TESTS=ON -DENABLE_COVERAGE=ON ..
    make unit-tests
    make coverage

The html report for the coverage can be found in the `build/coverage-results/` folder.
