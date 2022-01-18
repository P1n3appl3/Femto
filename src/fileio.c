#include "../inc/fileio.h"

extern int ENABLE_LINE_NUMS;
extern int LINE_NUM_WIDTH;

void openFile(char* filename) {
    free(E.filename);
    LINE_NUM_WIDTH = 0;
    E.filename = strdup(filename);
    FILE* fp = fopen(filename, "r");
    if (!fp) { return; }
    detectLang();
    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 &&
               (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
            --linelen;
        }
        char* temp = NULL;
        linelen = removeTabs(line, linelen, &temp);
        insertRow(E.numrows, temp, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

void saveFile() {
    if (E.filename == NULL) {
        if ((E.filename = prompt("Save as: %s (ESC to cancel)", NULL)) ==
            NULL) {
            setStatusMessage("Save aborted");
            return;
        }
        detectLang();
    }
    int len;
    char* buf = joinRows(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1 && write(fd, buf, len) == len) {
            close(fd);
            free(buf);
            E.dirty = 0;
            setStatusMessage("Wrote %d bytes to disk", len);
            return;
        }
        close(fd);
    }
    free(buf);
    setStatusMessage("Save failed! I/O error: %s", strerror(errno));
}

char* joinRows(int* buflen) {
    int len = 0;
    for (int i = 0; i < E.numrows; ++i) { len += E.row[i].size + 1; }
    *buflen = len;

    char* buf = malloc(len);
    char* p = buf;
    for (int i = 0; i < E.numrows; ++i) {
        memcpy(p, E.row[i].text, E.row[i].size);
        p += E.row[i].size;
        *p = '\n';
        ++p;
    }
    return buf;
}

void detectLang() {
    E.syntax = NULL;
    if (E.filename == NULL) { return; }
    char* ext = strrchr(E.filename, '.');
    if (ext == NULL) { return; }
    ++ext;
    for (int i = 0; (unsigned)i < NUM_SYNTAX; ++i) {
        struct editorSyntax* s = &HL_SETTINGS[i];
        if (strstr(s->filematch, ext) != NULL) {
            E.syntax = s;
            for (int i = 0; i < E.numrows; ++i) { renderRow(&E.row[i]); }
            return;
        }
    }
}

ssize_t removeTabs(char* in, ssize_t len, char** out) {
    int tabs = 0;
    for (int i = 0; i < len; ++i) { tabs += in[i] == '\t'; }
    *out = malloc(len + tabs * (E.tabSize - 1) + 1);
    int j = 0;
    for (int i = 0; i < len; ++i) {
        if (in[i] == '\t') {
            (*out)[j++] = ' ';
            while (j % E.tabSize != 0) { (*out)[j++] = ' '; }
        } else {
            (*out)[j++] = in[i];
        }
    }
    (*out)[j] = '\0';
    return j;
}
