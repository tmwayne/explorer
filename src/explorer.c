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

  // TODO: set dynamic column sizing
  Frame_T frame = Frame_init(
    16,         // col_width
    4,          // max_cols
    10          // max_rows
  );

  Data_T data = Data_file_init(
    file, // file
    1           // headers
  );

  data->open(data->args);
  data->load(data, frame, data->args);

  // data->fetch(data, frame, 0, 1, data->args);
 
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

    // TODO: create parse for this routine
    // Parse input
    switch (ch) {
      case 'j':
        if (cursor.row < frame->nrows) cursor.row++;
        break;
      case 'k':
        if (cursor.row > 0) cursor.row--;
        break;
      case 'h':
        if (cursor.col/frame->col_width > 0) 
          cursor.col -= frame->col_width;
        break;
      case 'l':
        if (cursor.col/frame->col_width < frame->ncols-1) 
          cursor.col += frame->col_width;
        else if (data->cursor.col < data->ncols-1) {
          data->fetch(data, frame, 0, 1, data->args);
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
