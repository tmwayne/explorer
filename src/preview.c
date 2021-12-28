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
#include "deque.h"
#include "frame.h"

void print_char_node(void **x, void *cl) {

  printf("%s\n", * (char **) x);

}

int main(int argc, char **argv) {

  // TODO: setup configparse

  // TODO: setup argparse
  if (argc != 2) {
    fprintf(stderr, "Usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *path = argv[1];

  // TODO: max_rows doesn't include header, make consistent
  // TODO: set dynamic column sizing
  Frame_T frame = Frame_init(
    16,         // col_width
    4,          // max_cols
    4           // max_rows
  );

  Data_T data = Data_file_init(
    path,       // path
    1           // headers
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
        if (cursor.row < frame->nrows) 
          cursor.row++;
        else if (data->shift(data, frame, 1, 0, data->args))
          Frame_print(frame);
        break;

      case 'k': // up
        if (cursor.row > 0) cursor.row--;
        else if (data->inframe.first_row > data->headers) {
          data->shift(data, frame, -1, 0, data->args);
          Frame_print(frame);
        }
        break;

      case 'h': // left
        if (frame_col > 0) 
          cursor.col -= frame->col_width;
        else if (data->inframe.first_col > 0) {
          data->shift(data, frame, 0, -1, data->args);
          Frame_print(frame);
        }
        break;

      case 'l': // right
        if (frame_col < frame->ncols-1) 
          cursor.col += frame->col_width;
        else if (data->inframe.last_col < data->ncols-1) {
          data->shift(data, frame, 0, 1, data->args);
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
