#ifndef TERMINAL_H
#define TERMINAL_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"

void die(const char* s);

void debugRead();

void enableRawMode();

void disableRawMode();

int getCursorPosition(int* rows, int* cols);

int getWindowSize(int* rows, int* cols);

void clearScreen();

int readKey();

#endif
