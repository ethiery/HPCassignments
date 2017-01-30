#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/**
 * Make sure MPI is initialized, and will be finalized at exit
 */
extern void check_MPI();

/**
 * Call fprintf on only one of the MPI processes.
 * @param stream fprintf first argument
 * @param format fprintf second argument
 * @param ...    fprintf variadic arguments
 */
extern void fprintf_once(FILE *stream, const char *format, ...);

#endif //UTILS_H
