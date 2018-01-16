#include "../inc/editor.h"

int main(int argc, char* argv[]){
    enableRawMode();
    initEditor();
    initDisplay();

    if (argc >= 2) {
        openFile(argv[1]);
    }

    setStatusMessage("^Q : Quit | ^S : Save | ^F : Find");

    while (1) {
        refresh();
        //debugRead();
        processKeypress();
    }

    return 0;
}
