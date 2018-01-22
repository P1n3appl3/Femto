#ifndef EDITOR_H
#define EDITOR_H

#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "display.h"
#include "fileio.h"
#include "terminal.h"

/**
 * Encapsulates a row of text including its position in the file and syntax
 * highlighting data.
 */
struct erow {
        int index;
        char* text;
        int size;
        char* hl;
        int unclosed;
};

typedef struct erow erow;

/**
 * Global object containing the state of the editor.
 */
struct editorConfig {
        int cx, oldcx, cy;
        int width, height;
        int scrollRow, scrollCol;
        erow* row;
        int numrows;
        int dirty;
        char* filename;
        int tabSize;
        char statusmsg[80];
        time_t messageTime;
        struct editorSyntax* syntax;
        struct termios orig_termios;
} E;

enum keys {
        BACKSPACE = 127,
        ARROW_UP = 1024,
        ARROW_DOWN,
        ARROW_LEFT,
        ARROW_RIGHT,
        PAGE_UP,
        PAGE_DOWN,
        HOME_KEY,
        END_KEY,
        DEL_KEY
};

/**
 * Initializes global editor object, destroying an old one of it existed.
 */
void initEditor();

/**
 * Displays a string to the user with printf style fomatting flags.
 */
void setStatusMessage(const char* fmt, ...);

/**
 * Displays a string to the user and lets them input a response, with an
 * optional callback function firing after each keypress.
 */
char* prompt(char* prompt, void (*callback)(char*, int));

/**
 * Callback function for the find operation. Handles moving between selections
 * with arrow keys and canceling or finishing the find operation.
 * @param query current search string
 * @param key   latest key pressed
 */
void findcb(char* query, int key);

/**
 * Prompts the user to search for a string in the file.
 */
void find();

/**
 * Creates a new row in the editor.
 * @param s   seed string for new row
 * @param len seed string length
 */
void insertRow(int at, char* s, size_t len);

void deleteRow(int at);

void freeRow(erow* row);

/**
 * Handles arrow key presses.
 */
void moveCursor(int key);

void rowInsertChar(erow* row, int at, char c);

void rowDeleteChar(erow* row, int at);

void rowAppendStr(erow* row, char* s, size_t len);

void insertNewline();

void insertChar(int c);

void deleteChar();

int countTabs(char* s, size_t len);

/**
 * Contains the high level logic for handling different kinds of keys (blocking).
 */
void processKeypress();

#endif
