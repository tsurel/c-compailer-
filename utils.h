#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/**
 * An header file for the utilities (utils) translation unit.
 */

/**
 * Copies a sub-string from the source parameter into the destination parameter,
 * starting from the given start index until the given end index (included).
 */
void subString(char *dest, const char *src, int start, int end);

#endif
