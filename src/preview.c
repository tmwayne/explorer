//
// -----------------------------------------------------------------------------
// preview.c
// -----------------------------------------------------------------------------
//
// Development version of Preview
//

#include <stdio.h>    // fprintf
#include <stdlib.h>   // exit, EXIT_FAILURE
#include <ncurses.h>
#include "argparse.h" // arguments, argp_parse
#include "preview.h"

void print_char_node(void **x, void *cl) {

  printf("%s\n", * (char **) x);

}

void yyerror(char *s) {

  fprintf(stderr, "%s\n", s);

}

// TODO: this is declared globally so it can be seen by bison
Frame_T frame;
Data_T data;
int done;

int main(int argc, char **argv) {

  // TODO: setup configparse

  // Command line arguments
  struct arguments arguments;
  arguments.headers = 1;
  arguments.delim = '|';
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  int col_width = 16;
  int max_cols = 4;
  int max_rows = 5;

  // TODO: determine appropriate line length
  // TODO: set dynamic column sizing
  // TODO: calculate # of rows and cols based on window
  frame = Frame_init(
    col_width,            // col_width
    max_cols,             // max_cols
    max_rows,             // max_rows
    arguments.headers     // headers
  );

  data = Data_file_init(
    arguments.path,       // path
    arguments.delim,      // delim
    arguments.headers     // headers
  );

  // TODO: check for error if unable to open
  data->open(data->args);
  data->load(data, frame, data->args);

  // data->shift(data, frame, 1, 0, data->args);
 
  initscr();
  cbreak(); // disable line buffering
  noecho(); // disable echo for getch
  curs_set(0); // hide cursor

  int ch;
  done = 1;

  Frame_print(frame);
  // TODO: move these lines to Frame_print
  move(frame->cursor.row, frame->cursor.col); // (row, col)
  chgat(frame->col_width-1, A_REVERSE, 0, NULL);

  // TODO: error checking?
  yyparse();

  endwin();

  data->close(data->args);

  free(data);
  free(frame);

}
