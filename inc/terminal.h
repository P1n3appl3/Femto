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

/**
 * Exits the program, printing a human readable error message.
 * To actually see these messages you have to disable smcup and rmcup in
 * enableRawMode().
 */
void die(const char* s);

/**
 * Prints raw byte values of characters from stdin (blocking).
 */
void debugRead();

/**
 * Messes with terminal settings to disable unwanted stuff like echoing.
 */
void enableRawMode();

/**
 * Resets terminal to normal settings and clears the screen (called on exit).
 */
void disableRawMode();

/**
 * Queries terminal for cursor x and y.
 * @return exit status
 */
int getCursorPosition(int* x, int* y);

/**
 * Queries terminal for width and height.
 * @return exit status
 */
int getWindowSize(int* rows, int* cols);

void clearScreen();

/**
 * Reads keypress and if needed parses escape sequences for special keys like
 * arrows, backspace, etc. (blocking).
 * @return keycode as int instead of char to allow for enum of special
 * keys
 */
int readKey();

#endif
