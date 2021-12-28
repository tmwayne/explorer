//
// -----------------------------------------------------------------------------
// explorer.c
// -----------------------------------------------------------------------------
//
// Development version of explorer
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
    fprintf(stderr, "Usage: %s file\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *file = argv[1];

  // TODO: max_rows doesn't include header, make consistent
  // TODO: set dynamic column sizing
  Frame_T frame = Frame_init(
    16,         // col_width
    4,          // max_cols
    4           // max_rows
  );

  Data_T data = Data_file_init(
    file, // file
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

    int frame_col_ind = cursor.col / frame->col_width;
    int frame_max_col_ind = frame->ncols - 1;
    int data_start_col_ind = data->cursor.col - (frame->ncols-1);
    int data_max_col_ind = data->ncols - 1;


    // TODO: create parse for this routine
    // Parse input
    switch (ch) {
      case 'j': // down
        if (cursor.row < frame->nrows) cursor.row++;
        else if (data->shift(data, frame, 1, 0, data->args))
          Frame_print(frame);
        break;
      case 'k': // up
        if (cursor.row > 0) cursor.row--;
        else if (data->cursor.row > frame->max_rows) {
          data->shift(data, frame, -1, 0, data->args);
          Frame_print(frame);
        }
        break;

      case 'h': // left
        if (frame_col_ind > 0) 
          cursor.col -= frame->col_width;
        else if (data_start_col_ind > 0) {
          data->shift(data, frame, 0, -1, data->args);
          Frame_print(frame);
        }
        break;

      case 'l': // right
        if (frame_col_ind < frame_max_col_ind) 
          cursor.col += frame->col_width;
        else if (data->cursor.col < data_max_col_ind) {
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
