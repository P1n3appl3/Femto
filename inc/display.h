#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>

#include "abuf.h"
#include "editor.h"

#define NUM_SYNTAX 1
#define SETTING_NUMBER (1<<0)
#define SETTING_STRING (1<<1)

struct erow;

struct editorSyntax {
        char* filetype;
        char* filematch;
        char* slCommentStart;
        char* mlCommentStart;
        char* mlCommentEnd;
        char** keywords;
        int flags;
};

struct editorSyntax HL_SETTINGS[NUM_SYNTAX];

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

void initDisplay();

int is_separator(int c);

void updateSyntax(struct erow* row);

void renderRow(struct erow* row);

int getColor(int hl);

int cxtorx(struct erow* row, int cx);

int rxtocx(struct erow* row, int rx);

void drawWelcome(abuf* ab);

void drawStatus(abuf* ab);

void drawMessageBar(abuf* ab);

void drawRows(abuf* ab);

void refresh();

#endif
