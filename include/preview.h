// 
// -----------------------------------------------------------------------------
// input-parser.h
// -----------------------------------------------------------------------------
//
// Input parser for Preview.
// Forward declares flex & bison types and functions
//
// Tyler Wayne © 2021
//

#ifndef INPUTPARSER_INCLUDED
#define INPUTPARSER_INCLUDED

#include "deque.h"
#include "frame.h"

// Interface to lexer
extern int yylineno;
void yyerror(char *s);
int yylex();
int yyparse();

// Program data
extern Frame_T frame;
extern Data_T data;

#endif // INPUTPARSER_INCLUDED
