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
  data->load(frame, data->args);
 
  initscr();
  cbreak(); // disable line buffering
  noecho(); // disable echo for getch
  curs_set(0); // hide cursor

  int ch, done = 1;

  Frame_print(frame);

  struct cursor cursor = frame->cursor; // alias the cursor

  do {

    move(cursor.x, cursor.y);
    chgat(frame->col_width-1, A_REVERSE, 0, NULL);

    refresh();    // render
    ch = getch(); // block on input

    chgat(frame->col_width-1, A_NORMAL, 0, NULL);

    // TODO: create parse for this routine
    // Parse input
    switch (ch) {
      case 'j':
        if (cursor.x < frame->nrows) cursor.x++;
        break;
      case 'k':
        if (cursor.x > 0) cursor.x--;
        break;
      case 'h':
        if (cursor.y/frame->col_width > 0) 
          cursor.y -= frame->col_width;
        break;
      case 'l':
        if (cursor.y/frame->col_width < frame->ncols-1) 
          cursor.y += frame->col_width;
        break;
      case 'q':
        done = 0;
        break;
    }
  } while (done);

  endwin();

  data->close(data->args);

}
