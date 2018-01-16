#ifndef ABUF_H
#define ABUF_H

#include <stdlib.h>
#include <string.h>

typedef struct abuf {
        char* b;
        int len;
} abuf;

#define ABUF_INIT {NULL, 0};

void abappend(abuf* ab, const char* s, int len);

void abfree(abuf* ab);

#endif
