#ifndef EDITOR_H
#define EDITOR_H

#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "display.h"
#include "fileio.h"
#include "terminal.h"

struct erow {
        int index;
        char* text;
        int size;
        char* hl;
        int unclosed;
};

typedef struct erow erow;

struct editorConfig {
        int cx, oldcx, cy;
        int width, height;
        int scrollRow, scrollCol;
        erow* row;
        int numrows;
        int dirty;
        char* filename;
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

void initEditor();

void setStatusMessage(const char* fmt, ...);

char* prompt(char* prompt, void (*callback)(char*, int));

void findcb(char* query, int key);

void find();

void insertRow(int at, char* s, size_t len);

void freeRow(erow* row);

void moveCursor(int key);

void rowInsertChar(erow* row, int at, char c);

void rowDeleteChar(erow* row, int at);

void rowAppendStr(erow* row, char* s, size_t len);

void deleteRow(int at);

void insertNewline();

void insertChar(int c);

void deleteChar();

int countTabs(char* s, size_t len);

void processKeypress();

#endif
