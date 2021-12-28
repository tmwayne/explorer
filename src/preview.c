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
#include "argparse.h"
#include "deque.h"
#include "frame.h"

void print_char_node(void **x, void *cl) {

  printf("%s\n", * (char **) x);

}

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

  // TODO: set dynamic column sizing
  // TODO: calculate # of rows and cols based on window
  Frame_T frame = Frame_init(
    col_width,            // col_width
    max_cols,             // max_cols
    max_rows,             // max_rows
    arguments.headers     // headers
  );

  Data_T data = Data_file_init(
    arguments.path,       // path
    arguments.delim,      // delim
    arguments.headers     // headers
  );

  data->open(data->args);
  data->load(data, frame, data->args);

  // data->shift(data, frame, 1, 0, data->args);
 
  initscr();
  cbreak(); // disable line buffering
  noecho(); // disable echo for getch
  curs_set(0); // hide cursor

  int ch, done = 1;

  Frame_print(frame);

  struct cursor cursor = frame->cursor; // alias the cursor

  do {

    move(cursor.row, cursor.col); // (row, col)
    chgat(frame->col_width-1, A_REVERSE, 0, NULL);

    refresh();    // render
    ch = getch(); // block on input

    chgat(frame->col_width-1, A_NORMAL, 0, NULL);

    int frame_col = cursor.col / frame->col_width;

    // TODO: create parse for this routine
    // Parse input
    switch (ch) {

      case 'j': // down
        if (cursor.row < frame->nrows + data->headers - 1) 
          cursor.row++;
        else if (data->shift_row(data, frame, 1, data->args))
          Frame_print(frame);
        break;

      case 'k': // up
        if (cursor.row > 0) cursor.row--;
        else if (data->inframe.first_row > 0) {
          data->shift_row(data, frame, -1, data->args);
          Frame_print(frame);
        }
        break;

      case 'h': // left
        if (frame_col > 0) 
          cursor.col -= frame->col_width;
        else if (data->inframe.first_col > 0) {
          data->shift_col(data, frame, -1, data->args);
          Frame_print(frame);
        }
        break;

      case 'l': // right
        if (frame_col < frame->ncols-1) 
          cursor.col += frame->col_width;
        else if (data->inframe.last_col < data->ncols-1) {
          data->shift_col(data, frame, 1, data->args);
          Frame_print(frame);
        }
        break;

      case 'q':
        done = 0;
        break;
    }
  } while (done);

  endwin();

  data->close(data->args);

}
