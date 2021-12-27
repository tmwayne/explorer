//
// -----------------------------------------------------------------------------
// frame.c
// -----------------------------------------------------------------------------
//

#include <stdlib.h>   // exit, EXIT_FAILURE
#include <string.h>   // strdup
#include <mem.h>      // NEW0, CALLOC, FREE
#include <cstrings.h> // get_line, get_tok_r
#include "deque.h"
#include "frame.h"

Frame_T Frame_init(int col_width, int max_cols, int max_rows) {

  Frame_T frame;
  NEW0(frame);

  frame->col_width = col_width;
  frame->max_cols = max_cols;
  frame->max_rows = max_rows;
  frame->ncols = 0;
  frame->nrows = 0;
  frame->cursor.x = 0;
  frame->cursor.y = 0;
  frame->headers = Deque_new();
  frame->data = Deque_new();

  return frame;

}

int Frame_print(Frame_T frame) {

  // on when to use erase() vs clear()
  // lists.gnu.org/archive/html/bug-ncurses/2014-01/msg00007.html
  erase();

  int start = 1;

  for (int c=0; c<frame->ncols; c++) {
    int text_start = c*frame->col_width+1;
    int text_width = frame->col_width-3;

    // Print headers
    mvaddnstr(0, text_start, Deque_get(frame->headers, c), text_width);
    if (c < frame->ncols-1)
      mvaddstr(0, (c+1)*frame->col_width-1, "|");
    
    // Print data
    Deque_T col = Deque_get(frame->data, c);

    for (int r=0, n=0; r<frame->nrows; r++, n++) {
      mvaddnstr(r+start, text_start, Deque_get(col, r), text_width);
      if (c < frame->ncols-1)
        mvaddstr(r+start, (c+1)*frame->col_width-1, "|");
    }
  }

  // TODO: return error code
  return 1;

}

typedef struct Data_file_args {
  char *file;
  FILE *fd;
} *Data_file_args;

int Data_file_open(void *args) {
  
  Data_file_args _args = args;

  // TODO: check for error
  _args->fd = fopen(_args->file, "r");
  
  // TODO: return error status
  return 1;

}

// int Data_file_load(Frame_T frame, FILE *fd) {
int Data_file_load(Frame_T frame, void *args) {

  Data_file_args _args = args;

  int err;

  char *line = CALLOC(256, sizeof(char));
  char *word = NULL, *saveptr = NULL;

  Deque_T col = NULL;
  int icol = 0;

  // Load the first line and create a deque for each column
  err = get_line(NULL, line, 255, _args->fd);
  if (err != E_OK) {
    fprintf(stderr, "Error reading in line\n");
    exit(EXIT_FAILURE);
  }

  // Load headers
  do {
    if (saveptr == NULL) word = get_tok_r(line, '|', &saveptr);
    if (word != NULL && frame->ncols < frame->max_cols) {
        col = Deque_new();
        Deque_addhi(frame->data, col); // add a new col
        Deque_addhi(frame->headers, strdup(word)); // add the header
        frame->ncols++;
    }
  } while ((word = get_tok_r(NULL, '|', &saveptr)) != NULL);

  // Load the remaining rows
  while ((err = get_line(NULL, line, 255, _args->fd)) == E_OK) {
    saveptr = NULL;
    icol = 0;
    do {
      if (saveptr == NULL) word = get_tok_r(line, '|', &saveptr);
      if (word != NULL && icol < frame->ncols) {
        col = Deque_get(frame->data, icol++);
        Deque_addhi(col, strdup(word)); // add the value for the row
      }
    } while ((word = get_tok_r(NULL, '|', &saveptr)) != NULL);
    frame->nrows++;
  }

  FREE(line);

  // TODO: return error status
  return 1;

}

int Data_file_close(void *args) {

  Data_file_args _args = args;

  // TODO: check for error
  fclose(_args->fd);
  
  // TODO: return error status
  return 1;

}

Data_T Data_file_init(char *file, int headers) {

  Data_T data;
  NEW0(data);

  Data_file_args args;
  NEW0(args);

  args->file = file;

  data->headers = headers;
  data->open = Data_file_open;
  data->load = Data_file_load;
  // data->fetch = NULL;
  data->close = Data_file_close;
  data->args = args;

  return data;

}


  
