#include "../inc/editor.h"

int main(int argc, char* argv[]) {
    enableRawMode();
    initEditor();
    initDisplay();

    if (argc >= 2) { openFile(argv[1]); }

    setStatusMessage(
        "^Q : Quit | ^S : Save | ^F : Find | ^O : Open | ^N : New");

    while (1) {
        refresh();
        processKeypress();
    }

    return 0;
}
