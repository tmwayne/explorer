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


void free_char_node(void **x, void *cl) {

  FREE(* (char **) x);

}

Frame_T Frame_init(int col_width, int max_cols, int max_rows) {

  Frame_T frame;
  NEW0(frame);

  frame->col_width = col_width;
  frame->max_cols = max_cols;
  frame->max_rows = max_rows;
  frame->ncols = 0;
  frame->nrows = 0;
  frame->cursor.row = 0;
  frame->cursor.col = 0;
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

int Data_file_load(Data_T data, Frame_T frame, void *args) {

  Data_file_args _args = args;

  int err;

  char *line = CALLOC(256, sizeof(char));
  char *word = NULL, *saveptr = NULL;

  Deque_T col = NULL;
  int icol = 0, irow = 0;

  // Load the first line and create a deque for each column
  err = get_line(NULL, line, 255, _args->fd);
  if (err != E_OK) {
    fprintf(stderr, "Error reading in line\n");
    exit(EXIT_FAILURE);
  }

  // Load headers
  for (
    word = get_tok_r(line, '|', &saveptr);
    word;
    word = get_tok_r(NULL, '|', &saveptr)
  ) {
    if (frame->ncols < frame->max_cols) {
      col = Deque_new();
      Deque_addhi(frame->data, col); // add a new col
      Deque_addhi(frame->headers, strdup(word)); // add the header
      frame->ncols++;
    }
    data->ncols++;
  }

  // data->nrows++;
  data->cursor.col = frame->ncols - 1;

  // TODO: save start of each line in an array

  // Load the remaining rows

  while ((err = get_line(NULL, line, 255, _args->fd)) == E_OK) {
    if (irow < frame->max_rows) {
      saveptr = NULL;
      for (
        word = get_tok_r(line, '|', &saveptr), icol=0;
        word && icol < frame->ncols;
        word = get_tok_r(NULL, '|', &saveptr), icol++
      ) {
        col = Deque_get(frame->data, icol);
        Deque_addhi(col, strdup(word)); // add the value for the row
      }
      frame->nrows++;
      irow++;
      // data->nrows++;
    }
  }

  // TODO: this assumes there is always a header row
  data->cursor.row = frame->nrows;

  FREE(line);

  // TODO: return error status
  return 1;

}

int shift_col(Data_T data, Frame_T frame, int n, FILE *fd) {

  char *line = CALLOC(256, sizeof(char));
  char *word = NULL, *saveptr = NULL;
  int err, icol, irow = 0, nrow = 0;
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int pop_ind, new_col_ind;

  if (n == 1) { // add col to the right
    pop_ind = 0;
    new_col_ind = data->cursor.col + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
    data->cursor.col++;
  } else {      // add col to the left
    pop_ind = Deque_length(frame->data)-1;
    new_col_ind = data->cursor.col - frame->ncols;
    pop = Deque_remhi;
    push = Deque_addlo;
    data->cursor.col--;
  }

  // 1. Free values in first column
  free(pop(frame->headers));

  Deque_map(Deque_get(frame->data, pop_ind), free_char_node, NULL);
  Deque_T col = pop(frame->data);
  Deque_free(&col);

  // 2. Add new column
  col = Deque_new();
  push(frame->data, col);

  // 3. add values based on row, col indices from data

  // TODO: move fd to correct line based on index instead of starting from beginning
  rewind(fd);

  // TODO: check for error here
  get_line(NULL, line, 255, fd);

  for (
    word = get_tok_r(line, '|', &saveptr), icol=0;
    word && icol < new_col_ind;
    word = get_tok_r(NULL, '|', &saveptr), icol++
  ) ;

  if (!word) return -1;

  push(frame->headers, strdup(word));

  while ((err = get_line(NULL, line, 255, fd)) == E_OK) {
    // TODO: this assumes headers
    if (irow >= data->cursor.row - frame->nrows && nrow < frame->max_rows) {
      saveptr = NULL;
      for (
        word = get_tok_r(line, '|', &saveptr), icol=0;
        word && icol < new_col_ind;
        word = get_tok_r(NULL, '|', &saveptr), icol++
      ) ;
      if (!word) return -1;
      Deque_addhi(col, strdup(word));
      nrow++;
    }
    irow++;
  }

  free(line);

  // TODO: return error status
  return 0;

}

int shift_row(Data_T data, Frame_T frame, int n, FILE *fd) {

  // 1. Set parameters
  char *line = CALLOC(256, sizeof(char));
  char *word = NULL, *saveptr = NULL;
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);

  // 3. Load data, fast forward to select right
  // 4. Push data onto each column
  
  int err;
  int irow = 0, icol = 0, i = 0;
  int new_row_ind;
  int col_start_ind = data->cursor.col - frame->ncols + 1;
  int col_end_ind = data->cursor.col;

  if (n == 1) { // add row on bottom (scroll down)
    new_row_ind = data->cursor.row + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) { // add row on top (scroll up)
    // TODO: make sure this is greater than 0
    new_row_ind = data->cursor.row - frame->nrows;
    pop = Deque_remhi;
    push = Deque_addlo;
  }

  // TODO: use row index
  rewind(fd);

  // Fast-forward to the correct row
  // while ((err = get_line(NULL, line, 255, fd))) {
  while (1) {
    err = get_line(NULL, line, 255, fd);
    if (err == E_OK) {
      if (irow++ < new_row_ind) continue;
      else break;
    } else if (irow == new_row_ind) return 0; // no new row
  }

  for (
    word = get_tok_r(line, '|', &saveptr), icol=0;
    word && icol <= col_end_ind;
    word = get_tok_r(NULL, '|', &saveptr), icol++
  ) {
    if (icol >= col_start_ind) {
      if (!word) return -1;
      Deque_T col = Deque_get(frame->data, i);
      free(pop(col));
      push(col, strdup(word));
      i++;
    }
  }

  if (n == 1) data->cursor.row++;
  else data->cursor.row--;
  
  return 1;

}

int Data_file_shift(Data_T data, Frame_T frame, int nrows, int ncols, void *args) {

  Data_file_args _args = args;

  int nadded = ncols ?
    shift_col(data, frame, ncols, _args->fd) :
    shift_row(data, frame, nrows, _args->fd);

  return nadded;

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
  data->shift = Data_file_shift;
  data->close = Data_file_close;
  data->args = args;

  return data;

}
