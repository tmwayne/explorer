// 
// -----------------------------------------------------------------------------
// strings.h
// -----------------------------------------------------------------------------
//
// Common string functions
//
// Tyler Wayne © 2021
//


#ifndef CSTRING_INCLUDED
#define CSTRING_INCLUDED

#include <stdio.h>  // FILE

enum cstring_error {
  E_OK = 0,
  E_NO_INPUT = -1,
  E_OVERFLOW = -2,
  E_SMALL_BUF = 3,
};

extern int      get_line(char *prompt, char *buf, size_t n, FILE *fd);
extern char    *get_tok_r(char *str, const char delim, char **saveptr);

extern int      strmatch(const char *str, const char *target);
extern int      strcasematch(const char *str, const char *target);
extern char    *strtrim(char *str);
extern void     strlower(char *);
extern int      extmatch(const char *path, const char *ext);
extern char    *pathcat(char *path1, char *path2);

#endif
