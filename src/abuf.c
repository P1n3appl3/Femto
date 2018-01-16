#include "../inc/abuf.h"

void abappend(abuf* ab, const char* s, int len){
    char* new = realloc(ab->b, ab->len + len);
    if (new == NULL) {
        return;
    }
    memcpy(&new[ab->len], s, len);
    ab->len += len;
    ab->b = new;
}

void abfree(abuf* ab){
    free(ab->b);
}
