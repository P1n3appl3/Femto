#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>

#include "abuf.h"
#include "editor.h"

#define NUM_SYNTAX 2
#define SETTING_NUMBER (1 << 0)
#define SETTING_STRING (1 << 1)

struct erow;

/**
 * Contains parameters for syntax highlighting of a given language.
 */
struct editorSyntax {
    char* filetype;
    char* filematch;
    char* slCommentStart;
    char* mlCommentStart;
    char* mlCommentEnd;
    char** keywords;
    int flags;
};

extern struct editorSyntax HL_SETTINGS[NUM_SYNTAX];

enum highlight {
    HL_NORMAL = 0,
    HL_NUMBER,
    HL_STRING,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_ESCAPE,
    HL_KEYWORD_1,
    HL_KEYWORD_2,
    HL_MATCH
};

/**
 * Defines syntax of the supported languages.
 */
void initDisplay();

int is_separator(int c);

/**
 * Performs highlighting on a given row.
 */
void renderRow(struct erow* row);

/**
 * Maps color enum to ansi codes.
 */
int getColor(int hl);

/**
 * Updates view of the editor based on cursor position.
 */
void scroll();

void drawWelcome(abuf* ab);

void drawStatus(abuf* ab);

void drawMessageBar(abuf* ab);

void drawRows(abuf* ab);

/**
 * Drives the drawing of the display.
 */
void refresh();

#endif
