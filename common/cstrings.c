//
// -----------------------------------------------------------------------------
// string.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdio.h>    // fgetc, fflush, fgets, printf, getchar
#include <stdlib.h>   // calloc
#include <limits.h>   // PATH_MAX
#include <ctype.h>    // isspace, tolower
#include <stdbool.h>  // bool, true, false
#include <string.h>   // strlen

#include "cstrings.h"  // cstring_error

int get_line(char *prompt, char *buf, size_t n, FILE *fd) {

  int c, extra;

  // Size zero or one cannot store enough, so don't even try
  // we need space for at least a newline and null-terminator
  if (n < 2) return E_SMALL_BUF;

  // Use stdin for NULL file descriptor
  if (!fd) fd = stdin;

  // Output prompt
  if (fd == stdin && prompt) {
    printf("%s", prompt);
    fflush(stdout);
  }

  // Get line with buffer overrun protection
  if (!fgets(buf, n, fd)) return E_NO_INPUT;

  // Catch possibility of \0 in the input stream
  size_t len = strlen(buf);
  if (len < 1) return E_NO_INPUT;

  // If it was too long, there'll be no newline. In that case,
  // flush to end of line so that excess doesn't affect the next call
  if (buf[len-1] != '\n') {
    extra = 0;
    while (((c = getchar()) != '\n') && (c != EOF)) extra = 1;
    return (extra == 1) ? E_OVERFLOW : E_OK;
  }

  // Otherwise remove newline and give string back to caller
  buf[len - 1] = '\0';
  return E_OK;

}

// get_tok_r is like strtok_r in string.h except that it
// doesn't include the newline character
char *get_tok_r(char *str, const char delim, char **saveptr) {

  if (*saveptr == NULL) *saveptr = str;

  char *field = *saveptr;
  bool in_quote = false;

  for ( int i=0; ; i++) {
    switch(field[i]) {
      case '\0':
        if (i == 0) return NULL;
        else {
          *saveptr += i;
          return field;
        }
      case '\n':
        field[i] = '\0';
        *saveptr += i;
        return field;
      case '"':
        in_quote = in_quote ? false : true;
        break;
      default:
        if (field[i]==delim && !in_quote) {
          field[i] = '\0';
          *saveptr += (i+1);
          return field;
        }
    }
  }

  return NULL;

}


int strmatch(const char *str, const char *target) {

  // assert(str && target);

  do {
    if (*str != *target) return 0;
  } while (*str++ && *target++);

  return 1;

}

int strcasematch(const char *str, const char *target) {

  if (strcasecmp(str, target)) return 0;
  else return 1; // return 1 if match

}

int extmatch(const char *path, const char *ext) {

  // assert(path && ext);

  int path_len = strlen(path);
  int ext_len = strlen(ext);

  if (ext_len == 0 || ext_len > path_len)
    return 0;

  path += (path_len - ext_len);

  return strmatch(path, ext);
  
}

char *strtrim(char *str) {

  // assert(str);
  char *start = NULL, *end = str;

  for ( ; *str ; str++) {
    if (!isspace(*str)) {
      if (!start) start = str;
      else end = str+1;
    }
  }
  *end = '\0';
  return start ? start : end;

}

void strlower(char *str) {
  // assert(str);
  for ( ; *str; str++) *str = tolower(*str);
}

// Concatenate two paths, ensuring there is proper
// directory delimiters (e.g., /this/is + some/path = /this/is/some/path)
char *pathcat(char *path1, char *path2) {

  // assert(path1 && path2);

  int path_max = PATH_MAX - 1; // Ensure we have room for null-terminator
  // char *path = CALLOC(PATH_MAX, sizeof(char));
  char *path = calloc(PATH_MAX, sizeof(char));
  char *head = path;

  // Copy first path
  for ( ; path_max && *path1; path_max--, path++, path1++)
    *path = *path1;

  // Ensure there is one and only one slash between paths
  // But don't add anything if one of the two paths are empty
  if (*head == '\0' || *path2 == '\0') ;
  else if (*(path1-1) != '/' && *path2 != '/')
    *path++ = '/';
  else if (*(path1-1) == '/' && *path2 == '/')
    path2++;

  // Copy second path
  for ( ; path_max && *path2; path_max--, path++, path2++)
    *path = *path2;

  // Add null-terminator
  *path = 0;

  return head;

}
