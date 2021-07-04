#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#define ERROR 1 /* A return value that indicates a failed operation. */
#define SUCCESS 2 /* A return value that indicates a successful operation. */

typedef unsigned char Code; /* Error code data type (can be SUCCESS/ERROR). */

Code isValid(const char *fileName);

#endif
