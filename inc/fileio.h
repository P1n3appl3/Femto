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

/**
 * Loads a file into the editor, removing newlines and tabs. If filename
 * doesn't exist, empties the editor.
 * @param filename relative path to file to be opened
 */
void openFile(char* filename);

/**
 * Attempts to write the editor's contents to a file. If the current file
 * hasn't been named yet the user is prompted to do so. A message is displayed
 * with the success or failure of the operation.
 */
void saveFile();

/**
 * Concatenates the contents of the editor to a single string in preparation to
 * write to a file, re-adding newlines.
 * @param  buflen length of the resulting string
 * @return        resulting string
 */
char* joinRows(int* buflen);

/**
 * Examines the extension of the current file to determine its type and if
 * recognized, sets the editor's syntax highlighting to that language.
 */
void detectLang();

/**
 * Replaces literal tabs with spaces in a string.
 * @param  in  input string
 * @param  len input string length
 * @param  out output string
 * @return     output string length
 */
ssize_t removeTabs(char* in, ssize_t len, char** out);

#endif
