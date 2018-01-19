#include "../inc/editor.h"

#define CTRLKEY(k) ((k) & 0x1f)

extern int TAB_SIZE;
extern int ENABLE_LINE_NUMS;
extern int LINE_NUM_WIDTH;

void initEditor(){
    for (int i = 0; i < E.numrows; ++i) {
        freeRow(&E.row[i]);
    }
    E.cx = 0;
    E.oldcx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.scrollRow = 0;
    E.scrollCol = 0;
    E.row = NULL;
    E.filename = NULL;
    E.dirty = 0;
    E.statusmsg[0] = '\0';
    E.messageTime = 0;
    E.syntax = NULL;
    if (getWindowSize(&E.height, &E.width) == -1) {
        die("getWindowSize");
    }
    E.height -= 2;
}

void setStatusMessage(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.messageTime = time(NULL);
}

char* prompt(char* prompt, void (*callback)(char*, int)){
    size_t bufsize = 80;
    char* buf = malloc(bufsize);
    size_t buflen = 0;
    buf[buflen] = '\0';

    while (1) {
        setStatusMessage(prompt, buf);
        refresh();
        int c = readKey();
        if (c == '\x1b' || c == CTRLKEY('q')) {
            setStatusMessage("");
            if (callback) {
                callback(buf, c);
            }
            free(buf);
            return NULL;
        }else if (c == '\r') {
            if (buflen > 0) {
                setStatusMessage("");
                if (callback) {
                    callback(buf, c);
                }
                return buf;
            }
        }else if (c == BACKSPACE || c == DEL_KEY || c == CTRLKEY('h')) {
            if (buflen > 0) {
                buf[--buflen] = '\0';
            }
        }else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
        if (callback) {
            callback(buf, c);
        }
    }
}

void findcb(char* query, int key){
    static int last = -1;
    static int dir = 1;

    if (key == '\r' || key == '\x1b' || key == CTRLKEY('q')) {
        last = -1;
        dir = 1;
        return;
    }else if (key == ARROW_UP || key == ARROW_LEFT) {
        dir = -1;
    }else if (key == ARROW_DOWN || key == ARROW_RIGHT) {
        dir = 1;
    }else {
        last = -1;
    }

    if (last == -1) {
        dir = 1;
    }
    int current = last;
    for (int i = 0; i < E.numrows; ++i) {
        current += dir;
        if (current == -1) {
            current = E.numrows - 1;
        }else if (current == E.numrows) {
            current = 0;
        }
        erow* row = &E.row[current];
        char* match = strstr(row->text, query);
        if (match) {
            last = current;
            E.cy = current;
            E.cx = match - row->text;
            E.scrollRow = E.numrows;
            break;
        }
    }
}

void find(){
    int save_cx = E.cx;
    int save_cy = E.cy;
    int save_scrollRow = E.scrollRow;
    int save_scrollCol = E.scrollCol;
    char* query = prompt("Find: %s (Arrows/Enter to select, ESC to cancel)", findcb);
    if (query) {
        free(query);
    }else {
        E.cx = save_cx;
        E.cy = save_cy;
        E.scrollRow = save_scrollRow;
        E.scrollCol = save_scrollCol;
    }
}

void insertRow(int at, char* s, size_t len){
    if (at > E.numrows) {
        return;
    }
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], (E.numrows - at) * sizeof(erow));
    for (int i = at + 1; i <= E.numrows; ++i) {
        ++E.row[i].index;
    }
    E.row[at].index = at;
    E.row[at].size = len;
    E.row[at].text = malloc(len + 1);
    memcpy(E.row[at].text, s, len);
    E.row[at].text[len] = '\0';
    E.row[at].hl = NULL;
    E.row[at].unclosed = 0;
    renderRow(&E.row[at]);
    ++E.numrows;
    ++E.dirty;
    if (ENABLE_LINE_NUMS) {
        int temp = E.numrows;
        for (int i = LINE_NUM_WIDTH; i > 0; --i) {
            temp /= 10;
        }
        if (temp != 0) {
            ++LINE_NUM_WIDTH;
        }
    }
}

void freeRow(erow* row){
    free(row->text);
    free(row->hl);
}

void moveCursor(int key){
    erow* row = E.cy >= E.numrows ? NULL : &E.row[E.cy];
    switch (key) {
    case ARROW_UP:
        if (E.cy > 0) {
            --E.cy;
            E.cx = E.oldcx;
        }
        break;
    case ARROW_DOWN:
        if (E.cy < E.numrows) {
            ++E.cy;
            if (E.cy < E.numrows) {
                E.cx = E.oldcx;
            }
        }
        break;
    case ARROW_LEFT:
        if (E.cx > 0) {
            int tabs = countTabs(row->text, row->size);
            --E.cx;
            while (E.cx < tabs * TAB_SIZE && E.cx % TAB_SIZE != 0) {
                --E.cx;
            }
        }else if (E.cy > 0) {
            --E.cy;
            E.cx = E.row[E.cy].size;
        }
        E.oldcx = E.cx;
        break;
    case ARROW_RIGHT:
        if (row && E.cx < row->size) {
            int tabs = countTabs(row->text, row->size);
            ++E.cx;
            while (E.cx < tabs * TAB_SIZE && E.cx % TAB_SIZE != 0) {
                ++E.cx;
            }
        }else if (row && E.cx == row->size) {
            ++E.cy;
            E.cx = 0;
        }
        E.oldcx = E.cx;
        break;
    }
    row = E.cy >= E.numrows ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
}

void rowInsertChar(erow* row, int at, char c){
    if (at > row->size) {
        at = row->size;
    }
    row->text = realloc(row->text, row->size + 2);
    memmove(&row->text[at + 1], &row->text[at], row->size - at + 1);
    ++row->size;
    row->text[at] = c;
    renderRow(row);
}

void rowDeleteChar(erow* row, int at){
    if (at >= row->size) {
        return;
    }
    memmove(&row->text[at], &row->text[at + 1], row->size - at);
    --row->size;
    renderRow(row);
    ++E.dirty;
}

void rowAppendStr(erow* row, char* s, size_t len){
    row->text = realloc(row->text, row->size + len + 1);
    memcpy(&row->text[row->size], s, len);
    row->size += len;
    row->text[row->size] = '\0';
    renderRow(row);
    ++E.dirty;
}

void deleteRow(int at){
    if (at >= E.numrows) {
        return;
    }
    freeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], (E.numrows - at - 1) * sizeof(erow));
    for (int i = at; i < E.numrows - 1; ++i) {
        --E.row[i].index;
    }
    --E.numrows;
    ++E.dirty;
    if (ENABLE_LINE_NUMS) {
        int temp = E.numrows;
        for (int i = LINE_NUM_WIDTH - 1; i > 0; --i) {
            temp /= 10;
        }
        if (temp == 0) {
            --LINE_NUM_WIDTH;
        }
    }
}

void insertNewline(){
    if (E.cx == 0) {
        insertRow(E.cy, "", 0);
    }else {
        erow* row = &E.row[E.cy];
        insertRow(E.cy + 1, &row->text[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->text[row->size] = '\0';
        renderRow(row);
    }
    E.cx = 0;
    int tabs = countTabs(E.row[E.cy].text, E.row[E.cy].size) * TAB_SIZE;
    ++E.cy;
    while (tabs) {
        insertChar(' ');
        --tabs;
    }
}

void insertChar(int c){
    ++E.dirty;
    if (E.cy == E.numrows) {
        insertRow(E.numrows, "", 0);
    }
    rowInsertChar(&E.row[E.cy], E.cx, c);
    ++E.cx;
    E.oldcx = E.cx;
}

void deleteChar(){
    if ((E.cx == 0 && E.cy == 0) || E.cy == E.numrows) {
        return;
    }
    erow* row = &E.row[E.cy];
    if (E.cx > 0) {
        int tabs = countTabs(row->text, row->size);
        rowDeleteChar(row, E.cx - 1);
        --E.cx;
        if (E.cx < tabs * TAB_SIZE) {
            while (E.cx % TAB_SIZE != 0) {
                rowDeleteChar(row, E.cx - 1);
                --E.cx;
            }
        }
    }else {
        E.cx = E.row[E.cy - 1].size;
        rowAppendStr(&E.row[E.cy - 1], row->text, row->size);
        deleteRow(E.cy);
        --E.cy;
    }
    E.oldcx = E.cx;
}

int countTabs(char* s, size_t len){
    int indent;
    for (indent = 0; indent < (int)len && s[indent] == ' '; ++indent) ;
    return indent / TAB_SIZE;
}

void processKeypress(){
    static int quit_attempt = 0;
    int c = readKey();

    switch (c) {
    case '\r':
        insertNewline();
        break;

    case '\t':
        do {
            insertChar(' ');
        } while (E.cx % TAB_SIZE != 0);
        break;

    case BACKSPACE:
    case DEL_KEY:
    case CTRLKEY('h'):
        if (c == DEL_KEY) {
            moveCursor(ARROW_RIGHT);
        }
        deleteChar();
        break;

    case '\x1b':
    case CTRLKEY('l'):
        break;

    case CTRLKEY('q'):
        if (!quit_attempt && E.dirty) {
            quit_attempt = 1;
            setStatusMessage("WARNING: Unsaved changes! Press CTRL-Q again to force quit.");
            return;
        }
        exit(0);
        break;

    case CTRLKEY('n'):
        if (!quit_attempt && E.dirty) {
            quit_attempt = 1;
            setStatusMessage("WARNING: Unsaved changes! Press CTRL-N again to discard.");
            return;
        }
        initEditor();
        break;

    case CTRLKEY('o'):
        if (!quit_attempt && E.dirty) {
            quit_attempt = 1;
            setStatusMessage("WARNING: Unsaved changes! Press CTRL-O again to discard.");
            return;
        }
        char* newFile = prompt("Open file: %s (ESC to cancel)", NULL);
        if (newFile != NULL) {
            initEditor();
            openFile(newFile);
        }
        break;

    case CTRLKEY('s'):
        saveFile();
        break;

    case CTRLKEY('f'):
        find();
        break;

    case CTRLKEY('r'):
        if (getWindowSize(&E.height, &E.width) == -1) {
            die("getWindowSize");
        }
        E.height -= 2;
        break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
    case ARROW_LEFT:
        moveCursor(c);
        break;

    case PAGE_UP:
    case PAGE_DOWN:
    {
        if (c == PAGE_UP) {
            E.cy = E.scrollRow;
        }else {
            E.cy = E.scrollRow + E.height - 1;
            if (E.cy > E.numrows) {
                E.cy = E.numrows;
            }
        }
        int times = E.height;
        while (--times) {
            moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
    }
    break;

    case HOME_KEY:
        E.cx = 0;
        E.oldcx = 0;
        break;

    case END_KEY:
        if (E.cy < E.numrows) {
            E.cx = E.row[E.cy].size;
            E.oldcx = E.cx;
        }
        break;

    default:
        insertChar(c);
        break;
    }
    quit_attempt = 0;
}
