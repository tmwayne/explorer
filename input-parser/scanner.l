/*
 * -----------------------------------------------------------------------------
 * lexer.l
 * -----------------------------------------------------------------------------
 *
 * Lexer for input-parser
 *
 * Tyler Wayne © 2021
 */

%option noyywrap nodefault yylineno

%{
#include <ncurses.h>
#include "parser.h"
// #include "y.tab.h"

#define YY_INPUT(buf, result, max_size) { \
  int c = getch(); \
  result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
  }
%}

%%

h                       { return LEFT; }
l                       { return RIGHT; }
j                       { return DOWN; }
k                       { return UP; }
.                       { return OTHER; }

%%

