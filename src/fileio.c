#include "../inc/fileio.h"

void openFile(char* filename) {
    free(E.filename);
    E.filename = strdup(filename);
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return;
    }
    detectLang();
    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
            --linelen;
        }
        insertRow(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

void saveFile(){
    if (E.filename == NULL) {
        if ((E.filename = prompt("Save as: %s (ESC to cancel)", NULL)) == NULL) {
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

char* joinRows(int* buflen){
    int len = 0;
    for (int i = 0; i < E.numrows; ++i) {
        len += E.row[i].size + 1;
    }
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
