#include "../inc/display.h"
struct editorSyntax HL_SETTINGS[NUM_SYNTAX];

int ENABLE_LINE_NUMS = 1;
int LINE_NUM_WIDTH = 0;
int MESSAGE_TIMER = 3;

char* ckw[] = {
    "auto",     "break",    "case",     "char",     "const",     "continue",
    "default",  "do",       "double",   "else",     "enum",      "extern",
    "float",    "for",      "goto",     "if",       "int",       "long",
    "register", "return",   "short",    "signed",   "sizeof",    "static",
    "struct",   "switch",   "typedef",  "union",    "unsigned",  "void",
    "volatile", "while",    "getchar|", "putchar|", "printf|",   "scanf|",
    "strcat|",  "strcmp|",  "strcpy|",  "isdigit|", "isalpha|",  "isalnum|",
    "islower|", "isupper|", "acos|",    "asin|",    "atan|",     "cos|",
    "exp|",     "fabs|",    "sqrt|",    "time|",    "difftime|", "clock|",
    "malloc|",  "realloc|", "rand|",    "srand|",   NULL};

char* pykw[] = {
    "and",    "assert",      "break",      "class",  "continue",   "def",
    "del",    "elif",        "else",       "except", "exec",       "finally",
    "for",    "from",        "global",     "if",     "import",     "in",
    "is",     "lambda",      "not",        "or",     "pass",       "print",
    "raise",  "return",      "try",        "while",  "abs|",       "all|",
    "any|",   "bin|",        "bool|",      "char|",  "cmp|",       "dict|",
    "dir|",   "divmod|",     "enumerate|", "eval|",  "file|",      "filter|",
    "float|", "format|",     "hash|",      "help|",  "hex|",       "id|",
    "input|", "int|",        "iter|",      "len|",   "list|",      "long|",
    "map|",   "max|",        "min|",       "next|",  "oct|",       "open|",
    "ord|",   "pow|",        "print|",     "range|", "raw_input|", "reduce|",
    "repr|",  "reversed|",   "set|",       "slice|", "sorted|",    "str|",
    "sum|",   "super|",      "tuple|",     "type|",  "unicode|",   "xrange|",
    "zip|",   "__import__|", NULL};

void initDisplay() {
    struct editorSyntax* s = &HL_SETTINGS[0];
    s->filetype = "C";
    s->filematch = "c h cpp";
    s->slCommentStart = "//";
    s->mlCommentStart = "/*";
    s->mlCommentEnd = "*/";
    s->keywords = ckw;
    s->flags = SETTING_NUMBER | SETTING_STRING;
    s = &HL_SETTINGS[1];
    s->filetype = "Python";
    s->filematch = "py";
    s->slCommentStart = "#";
    s->mlCommentStart = NULL;
    s->mlCommentEnd = NULL;
    s->keywords = pykw;
    s->flags = SETTING_NUMBER | SETTING_STRING;
}

int is_separator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void renderRow(struct erow* row) {
    row->hl = realloc(row->hl, row->size);
    memset(row->hl, HL_NORMAL, row->size);
    if (E.syntax == NULL) { return; }

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
    while (i < row->size) {
        char c = row->text[i];
        char prev = i == 0 ? HL_NORMAL : row->hl[i - 1];

        if (scslen && !instr && !incom) {
            if (!strncmp(&row->text[i], scs, scslen)) {
                memset(&row->hl[i], HL_COMMENT, row->size - i);
                break;
            }
        }

        if (mcslen && mcelen && !instr) {
            if (incom) {
                row->hl[i] = HL_MLCOMMENT;
                if (!strncmp(mce, &row->text[i], mcelen)) {
                    memset(&row->hl[i], HL_MLCOMMENT, mcelen);
                    i += mcelen;
                    incom = 0;
                    sep = 1;
                    continue;
                } else {
                    ++i;
                    continue;
                }
            } else if (!strncmp(mcs, &row->text[i], mcslen)) {
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
            } else {
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
                if (!strncmp(&row->text[i], kw[j], kwlen) &&
                    is_separator(row->text[i + kwlen])) {
                    memset(&row->hl[i], kw2 ? HL_KEYWORD_2 : HL_KEYWORD_1,
                           kwlen);
                    i += kwlen;
                    break;
                }
            }
            if (kw[j] != NULL) { continue; }
        }

        sep = is_separator(c);
        ++i;
    }

    int changed = incom != row->unclosed;
    row->unclosed = incom;
    if (changed && row->index + 1 < E.numrows) {
        renderRow(&E.row[row->index + 1]);
    }
}

int getColor(int hl) {
    switch (hl) {
    case HL_NUMBER: return 31;
    case HL_MATCH: return 34;
    case HL_STRING: return 35;
    case HL_ESCAPE: return 34;
    case HL_COMMENT: return 36;
    case HL_MLCOMMENT: return 36;
    case HL_KEYWORD_1: return 32;
    case HL_KEYWORD_2: return 33;
    default: return 37;
    }
}

void scroll() {
    if (E.cy < E.scrollRow) { E.scrollRow = E.cy; }
    if (E.cy >= E.scrollRow + E.height) { E.scrollRow = (E.cy - E.height) + 1; }
    if (E.cx < E.scrollCol) { E.scrollCol = E.cx; }
    if (E.cx >= E.scrollCol + E.width - LINE_NUM_WIDTH) {
        E.scrollCol = (E.cx - (E.width - LINE_NUM_WIDTH)) + 1;
    }
}

void drawWelcome(abuf* ab) {
    char welcome[] = "Welcome to Femto!";
    int welcomelen = sizeof(welcome);
    if (welcomelen > E.width) { welcomelen = E.width; }
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

void drawStatus(abuf* ab) {
    abappend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    char* temp;
    if (E.filename) {
        temp = &E.filename[strlen(E.filename) - 20];
        if (strlen(E.filename) < 20) { temp = E.filename; }
    }
    snprintf(status, sizeof(status), "%s - %d lines %s",
             E.filename ? temp : "[No Name]", E.numrows,
             E.dirty ? "(modified)" : "");
    int len = strlen(status);
    int rlen =
        snprintf(rstatus, sizeof(rstatus), "Lang-%s | %d:%d",
                 E.syntax ? E.syntax->filetype : "???", E.cy + 1, E.cx + 1);
    if (len > E.width) { len = E.width; }
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

void drawMessageBar(abuf* ab) {
    abappend(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.width) { msglen = E.width; }
    if (msglen && time(NULL) - E.messageTime < MESSAGE_TIMER) {
        abappend(ab, E.statusmsg, msglen);
    }
}

void drawRows(abuf* ab) {
    for (int i = 0; i < E.height; ++i) {
        int currentRow = E.scrollRow + i;
        if (currentRow >= E.numrows) {
            if (E.numrows == 0 && i == E.height / 3) {
                drawWelcome(ab);
            } else {
                abappend(ab, "~", 1);
            }
        } else {
            char lineNum[16];
            if (ENABLE_LINE_NUMS && LINE_NUM_WIDTH) {
                snprintf(lineNum, sizeof(lineNum), "%08d", E.scrollRow + i + 1);
                char buf[32];
                snprintf(buf, sizeof(buf), "\x1b[7m%s\x1b[m",
                         &lineNum[8 - LINE_NUM_WIDTH]);
                abappend(ab, buf, strlen(buf));
            }
            int len = E.row[currentRow].size - E.scrollCol;
            if (len < 0) { len = 0; }
            if (len > E.width - LINE_NUM_WIDTH) {
                len = E.width - LINE_NUM_WIDTH;
            }
            int color = -1;
            char* c = &E.row[currentRow].text[E.scrollCol];
            char* hl = &E.row[currentRow].hl[E.scrollCol];
            for (int j = 0; j < len; ++j) {
                if (iscntrl(c[j])) {
                    char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                    abappend(ab, "\x1b[7m", 4);
                    abappend(ab, &sym, 1);
                    abappend(ab, "\x1b[m", 3);
                    if (hl[j] != -1) {
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm",
                                            getColor(hl[j]));
                        abappend(ab, buf, clen);
                    }
                    continue;
                }
                if (hl[j] == HL_NORMAL) {
                    color = -1;
                    abappend(ab, "\x1b[39m", 5);
                } else if (hl[j] != color) {
                    color = hl[j];
                    char buf[16];
                    int clen =
                        snprintf(buf, sizeof(buf), "\x1b[%dm", getColor(hl[j]));
                    abappend(ab, buf, clen);
                }
                abappend(ab, &c[j], 1);
            }
            abappend(ab, "\x1b[39m", 5);
        }
        abappend(ab, "\x1b[K\r\n", 5);
    }
}

void refresh() {
    scroll();
    abuf ab = ABUF_INIT;
    abappend(&ab, "\x1b[?25l", 6);
    abappend(&ab, "\x1b[H", 3);
    drawRows(&ab);
    drawStatus(&ab);
    drawMessageBar(&ab);
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.scrollRow + 1,
             E.cx - E.scrollCol + 1 + (ENABLE_LINE_NUMS ? LINE_NUM_WIDTH : 0));
    abappend(&ab, buf, strlen(buf));
    abappend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abfree(&ab);
}
