/*
 * Error handling routines
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int errors;
int warnings;
int dbflag = 1;

void fatal(char *fmt,int i, ...)
{
        va_list valist;
        va_start(valist, i);

        fprintf(stderr, "\nFatal error: ");
        vfprintf(stderr, fmt, valist);
        fprintf(stderr, "\n");

        va_end(valist);
        exit(1);
}

void error(char *fmt,int i, ...)
{
        va_list valist;
        va_start(valist, i);

        fprintf(stderr, "\nError: ");
        vfprintf(stderr, fmt, valist);;
        fprintf(stderr, "\n");
        errors++;

        va_end(valist);
}

void warning(char *fmt,int i, ...)
{
        va_list valist;
        va_start(valist, i);

        fprintf(stderr, "\nWarning: ");
        vfprintf(stderr, fmt, valist);
        fprintf(stderr, "\n");
        warnings++;

        va_end(valist);
}

void debug(char *fmt,int i, ...)
{
        va_list valist;
        va_start(valist, i);

        if(!dbflag) return;
        fprintf(stderr, "\nDebug: ");
        vfprintf(stderr, fmt, valist);
        fprintf(stderr, "\n");

        va_end(valist);
}
