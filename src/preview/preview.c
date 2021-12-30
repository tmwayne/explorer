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
#include "errorcodes.h"

void print_char_node(void **x, void *cl) {

  printf("%s\n", * (char **) x);

}

void yyerror(char *s) {

  fprintf(stderr, "%s\n", s);

}

// TODO: make parser reentrant to avoid global variables
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
 
  initscr();
  cbreak(); // disable line buffering
  noecho(); // disable echo for getch
  curs_set(0); // hide cursor

  int col_width = 16;
  int max_rows, max_cols;
  getmaxyx(stdscr, max_rows, max_cols);
  max_cols /= col_width;

  // TODO: determine appropriate column width
  frame = Frame_init(
    col_width,            // col_width
    max_cols,             // max_cols
    max_rows-1,           // max_rows
    arguments.headers     // headers
  );

  // data = Data_file_init(
  data = Data_mmap_init(
    arguments.path,       // path
    arguments.delim,      // delim
    arguments.headers     // headers
  );


  // TODO: check for error if unable to open
  data->open(data->args);
  data->load(data, frame, data->args);

  // data->shift(data, frame, 1, 0, data->args);

  done = 1;

  Frame_print(frame);
  // TODO: move these lines to Frame_print
  move(frame->cursor.row, frame->cursor.col); // (row, col)
  chgat(frame->col_width-1, A_REVERSE, 0, NULL);

  // TODO: error checking?
  yyparse();

  endwin();

  data->close(data->args);

  // Data_file_free(&data);
  Data_mmap_free(&data);
  Frame_free(&frame);

}
