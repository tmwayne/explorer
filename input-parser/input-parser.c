//
// -----------------------------------------------------------------------------
// input-parser.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdio.h>
#include "preview.h"
#include "parser.h"

void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

void input_parser(FILE *fd) {

  yyin = fd;
  yyparse();

}
