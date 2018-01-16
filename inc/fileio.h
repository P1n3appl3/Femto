#ifndef FILEIO_H
#define FILEIO_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

void openFile(char* filename);

void saveFile();

char* joinRows(int* buflen);

#endif
