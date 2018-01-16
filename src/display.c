#include "../inc/display.h"

extern int LINE_NUM_WIDTH;

char* ckw[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL
};

void initDisplay(){
    struct editorSyntax* s = &HL_SETTINGS[0];
    s->filetype = "C";
    s->filematch = "c h cpp";
    s->slCommentStart = "//";
    s->mlCommentStart = "/*";
    s->mlCommentEnd = "*/";
    s->keywords = ckw;
    s->flags = SETTING_NUMBER | SETTING_STRING;
}

int is_separator(int c){
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void updateSyntax(struct erow* row){
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);
    if (E.syntax == NULL) {
        return;
    }

    int sep = 1;
    char instr = 0;
    int incom = row->index > 0 && E.row[row->index - 1].unclosed;
    char* scs = E.syntax->slCommentStart;
    char* mcs = E.syntax->mlCommentStart;
    char* mce = E.syntax->mlCommentEnd;
    int scslen = scs ? strlen(scs) : 0;
    int mcslen = mcs ? strlen(mcs) : 0;
    int mcelen = mce ? strlen(mcs) : 0;
    char** kw = E.syntax->keywords;
    int i = 0;
    while (i < row->rsize) {
        char c = row->render[i];
        char prev = i == 0 ? HL_NORMAL : row->hl[i - 1];

        if (scslen && !instr && !incom) {
            if (!strncmp(&row->render[i], scs, scslen)) {
                memset(&row->hl[i], HL_COMMENT, row->rsize - i);
                break;
            }
        }

        if (mcslen && mcelen && !instr) {
            if (incom) {
                row->hl[i] = HL_MLCOMMENT;
                if (!strncmp(mce, &row->render[i], mcelen)) {
                    memset(&row->hl[i], HL_MLCOMMENT, mcelen);
                    i += mcelen;
                    incom = 0;
                    sep = 1;
                    continue;
                }else {
                    ++i;
                    continue;
                }
            }else if (!strncmp(mcs, &row->render[i], mcslen)) {
                memset(&row->hl[i], HL_MLCOMMENT, mcslen);
                i += mcslen;
                incom = 1;
                continue;
            }
        }

        if (E.syntax->flags & SETTING_STRING) {
            if (instr) {
                if (c == '\\') {
                    row->hl[i] = HL_ESCAPE;
                    row->hl[i + 1] = HL_ESCAPE;
                    i += 2;
                    continue;
                }
                if (c == instr) {
                    instr = 0;
                    sep = 1;
                }
                row->hl[i] = HL_STRING;
                ++i;
                continue;
            }else{
                if (c == '"' || c == '\'') {
                    instr = c;
                    row->hl[i] = HL_STRING;
                    ++i;
                    continue;
                }
            }
        }

        if (E.syntax->flags & SETTING_NUMBER) {
            if ((isdigit(c) && (sep || prev == HL_NUMBER)) ||
                (prev == HL_NUMBER && c == '.')) {
                row->hl[i] = HL_NUMBER;
                sep = 0;
                ++i;
                continue;
            }
        }

        if (sep) {
            int j;
            for (j = 0; kw[j]; ++j) {
                int kwlen = strlen(kw[j]);
                int kw2 = kw[j][kwlen - 1] == '|';
                kwlen -= kw2;
                if (!strncmp(&row->render[i], kw[j], kwlen) &&
                    is_separator(row->render[i + kwlen])) {
                    memset(&row->hl[i], kw2 ? HL_KEYWORD_2 : HL_KEYWORD_1, kwlen);
                    i += kwlen;
                    break;
                }
            }
            if (kw[j] != NULL) {
                continue;
            }
        }

        sep = is_separator(c);
        ++i;
    }

    int changed = incom != row->unclosed;
    row->unclosed = incom;
    if (changed && row->index + 1 < E.numrows) {
        updateSyntax(&E.row[row->index + 1]);
    }
}

int getColor(int hl){
    switch (hl) {
    case HL_NUMBER:
        return 31;
    case HL_MATCH:
        return 34;
    case HL_STRING:
        return 35;
    case HL_ESCAPE:
        return 34; // find cyan
    case HL_COMMENT:
        return 36;
    case HL_MLCOMMENT:
        return 36;
    case HL_KEYWORD_1:
        return 33;
    case HL_KEYWORD_2:
        return 32;
    default:
        return 37;
    }
}

void renderRow(struct erow* row){
    int tabs = 0;
    for (int i = 0; i < row->size; ++i) {
        tabs += row->text[i] == '\t';
    }
    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_SIZE - 1) + 1);
    int j = 0;
    for (int i = 0; i < row->size; ++i) {
        if (row->text[i] == '\t') {
            row->render[j++] = ' ';
            while (j % TAB_SIZE != 0) {
                row->render[j++] = ' ';
            }
        }else {
            row->render[j++] = row->text[i];
        }
    }
    row->render[j] = '\0';
    row->rsize = j;
    updateSyntax(row);
}

int cxtorx(struct erow* row, int cx){
    int rx = 0;
    for (int i = 0; i < cx; ++i) {
        if (row->text[i] == '\t') {
            rx += (TAB_SIZE - 1) - (rx % TAB_SIZE);
        }
        ++rx;
    }
    return rx;
}

int rxtocx(struct erow* row, int rx){
    int cur = 0;
    int cx = 0;
    for (; cx < row->size; ++cx) {
        if (row->text[cx] == '\t') {
            cur += (TAB_SIZE - 1) - (rx % TAB_SIZE);
        }
        ++cur;
        if (cur > rx) {
            return cx;
        }
    }
    return cx;
}

void drawWelcome(abuf* ab){
    char welcome[] = "Welcome to Femto!";
    int welcomelen = sizeof(welcome);
    if (welcomelen > E.width) {
        welcomelen = E.width;
    }
    int padding = (E.width - welcomelen) / 2;
    if (padding) {
        abappend(ab, "~", 1);
        --padding;
        while (padding) {
            abappend(ab, " ", 1);
            --padding;
        }
    }
    abappend(ab, welcome, welcomelen);
}

void drawStatus(abuf* ab){
    abappend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    char* temp;
    if (E.filename) {
        temp = &E.filename[strlen(E.filename) - 20];
        if (strlen(E.filename) < 20) {
            temp = E.filename;
        }
    }
    snprintf(status, sizeof(status), "%s - %d lines %s",
             E.filename ? temp : "[No Name]", E.numrows,
             E.dirty ? "(modified)" : "");
    int len = strlen(status);
    int rlen = snprintf(rstatus, sizeof(rstatus), "Lang-%s | %d:%d",
                        E.syntax ? E.syntax->filetype : "???", E.cy + 1, E.cx + 1);
    if (len > E.width) {
        len = E.width;
    }
    abappend(ab, status, len);
    while (len < E.width) {
        if (E.width - len == rlen) {
            abappend(ab, rstatus, rlen);
            break;
        }
        abappend(ab, " ", 1);
        ++len;
    }
    abappend(ab, "\x1b[m", 3);
    abappend(ab, "\r\n", 2);
}

void drawMessageBar(abuf* ab){
    abappend(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.width) {
        msglen = E.width;
    }
    if (msglen && time(NULL) - E.messageTime < MESSAGE_TIMER) {
        abappend(ab, E.statusmsg, msglen);
    }
}

void drawRows(abuf* ab){
    for (int i = 0; i < E.height; ++i) {
        int currentRow = E.scrollRow + i;
        if (currentRow >= E.numrows) {
            if (E.numrows == 0 && i == E.height / 3) {
                drawWelcome(ab);
            }else {
                abappend(ab, "~", 1);
            }
        }else{
            char lineNum[16];
            snprintf(lineNum, sizeof(lineNum), "%08d", i + 1);
            char buf[32];
            snprintf(buf, sizeof(buf), "\x1b[7m%s\x1b[m", &lineNum[8 - LINE_NUM_WIDTH]);
            abappend(ab, buf, strlen(buf));
            int len = E.row[currentRow].rsize - E.scrollCol;
            if (len < 0) {
                len = 0;
            }
            if (len > E.width) {
                len = E.width;
            }
            int color = -1;
            char* c = &E.row[currentRow].render[E.scrollCol];
            char* hl = &E.row[currentRow].hl[E.scrollCol];
            for (int j = 0; j < len; ++j) {
                if (iscntrl(c[j])) {
                    char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                    abappend(ab, "\x1b[7m", 4);
                    abappend(ab, &sym, 1);
                    abappend(ab, "\x1b[m", 3);
                    if (hl[j] != -1) {
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", getColor(hl[j]));
                        abappend(ab, buf, clen);
                    }
                    continue;
                }
                if (hl[j] == HL_NORMAL) {
                    color = -1;
                    abappend(ab, "\x1b[39m", 5);
                }else if (hl[j] != color) {
                    color = hl[j];
                    char buf[16];
                    int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", getColor(hl[j]));
                    abappend(ab, buf, clen);
                }
                abappend(ab, &c[j], 1);
            }
            abappend(ab, "\x1b[39m", 5);
        }
        abappend(ab, "\x1b[K\r\n", 5);
    }
}

void refresh(){
    scroll();
    abuf ab = ABUF_INIT;
    abappend(&ab, "\x1b[?25l", 6);
    abappend(&ab, "\x1b[H", 3);
    drawRows(&ab);
    drawStatus(&ab);
    drawMessageBar(&ab);
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.scrollRow + 1, E.rx - E.scrollCol + 1 + LINE_NUM_WIDTH);
    abappend(&ab, buf, strlen(buf));
    abappend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abfree(&ab);
}
