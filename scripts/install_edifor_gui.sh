#!/bin/sh

gcc -O2 -Wall -I./include/ -I./usr/local/include/ $(pkg-config --cflags vte-2.91) ./src/EdiforGUI.c -o /bin/ediforg $(pkg-config --libs vte-2.91)
