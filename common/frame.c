//
// -----------------------------------------------------------------------------
// frame.c
// -----------------------------------------------------------------------------
//

#include <stdlib.h>   // exit, EXIT_FAILURE
#include <string.h>   // strdup
#include "mem.h"      // NEW0, CALLOC, FREE
#include "cstrings.h" // get_line, get_tok_r
#include "deque.h"
#include "frame.h"

#define LINE_LEN 8192

#define GET_TOK_R(line, delim, saveptr) \
  (get_tok_r( *saveptr ? NULL : line, delim, saveptr))

static void free_char_node(void **x, void *cl) {

  FREE(* (char **) x);

}

Frame_T Frame_init(int col_width, int max_cols, int max_rows, int headers) {

  Frame_T frame;
  NEW0(frame);

  frame->col_width = col_width;
  frame->max_cols = max_cols;
  frame->max_rows = max_rows - (headers ? 1 : 0);
  frame->headers = headers ? Deque_new() : NULL;
  frame->data = Deque_new();

  return frame;

}

int Frame_print(Frame_T frame) {

  // on when to use erase() vs clear()
  // lists.gnu.org/archive/html/bug-ncurses/2014-01/msg00007.html
  erase();

  int headers = frame->headers ? 1 : 0;

  for (int icol=0; icol<frame->ncols; icol++) {
    int text_start = icol*frame->col_width+1;
    int text_width = frame->col_width-3;

    // Print headers
    if (frame->headers) {
      mvaddnstr(0, text_start, Deque_get(frame->headers, icol), text_width);
      if (icol < frame->ncols-1)
        mvaddstr(0, (icol+1)*frame->col_width-1, "|");
    }
    
    // Print data
    Deque_T col = Deque_get(frame->data, icol);

    for (int irow=0, n=0; irow<frame->nrows; irow++, n++) {
      mvaddnstr(irow + headers, text_start, Deque_get(col, irow), text_width);
      if (icol < frame->ncols-1) // print for all but the last column
        mvaddstr(irow + headers, (icol+1)*frame->col_width-1, "|");
    }
  }

  // TODO: return error code
  return 1;

}

typedef struct file_args {
  char *path;
  char delim;
  FILE *fd;
} *file_args;

int file_open(void *args) {

  file_args _args = args;

  _args->fd = fopen(_args->path, "r");

  // TODO: return error code: unable to open file
  if (_args->fd) return 0;
  
  // TODO: return error status
  return 1;

}

int file_load(Data_T data, Frame_T frame, void *args) {

  FILE *fd = ((file_args) args)->fd;
  char delim = ((file_args) args)->delim;

  char *line = CALLOC(LINE_LEN, sizeof(char));
  char *word = NULL, *saveptr = NULL;
  Deque_T col = NULL;

  // Initialize data columns, column counts, and headers

  // TODO: add error code: unable to read first line
  if (get_line(NULL, line, 255, fd) != E_OK) return 0;

  int icol = 0;
  while ((word = GET_TOK_R(line, delim, &saveptr))) {
    if (icol < frame->max_cols) {
      Deque_addhi(frame->data, Deque_new());
      frame->ncols++;
      if (data->headers) 
        Deque_addhi(frame->headers, strdup(word)); // add the header
    } 
    data->ncols++;
    icol++;
  }

  // TODO: save start of each line in an array

  if (!data->headers) rewind(fd);
  
  // Load data
  int irow = 0;
  while (get_line(NULL, line, 255, fd) == E_OK) {
    if (irow < frame->max_rows) {
      saveptr = NULL;
      icol = 0;
      while ((word = GET_TOK_R(line, delim, &saveptr)) && icol < frame->max_cols) {
        col = Deque_get(frame->data, icol);
        Deque_addhi(col, strdup(word)); // add the value for the row
        icol++;
      }
      frame->nrows++;
      irow++;
    }
  }

  data->inframe.last_col = frame->ncols - 1;
  data->inframe.last_row = frame->nrows - 1;

  FREE(line);

  // TODO: return error status
  return 1;

}

int file_shift_col(Data_T data, Frame_T frame, int n, void *args) {

  FILE *fd = ((file_args) args)->fd;
  char delim = ((file_args) args)->delim;

  char *line = CALLOC(LINE_LEN, sizeof(char));
  char *word = NULL, *saveptr = NULL;
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int pop_ind, new_col_ind;

  if (n == 1) { // add col to the right
    pop_ind = 0;
    new_col_ind = data->inframe.last_col + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) {      // add col to the left
    pop_ind = Deque_length(frame->data)-1;
    new_col_ind = data->inframe.first_col - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  // TODO: return error code: invalid n
  } else return 0;

    // 1. Free values in first column
  Deque_map(Deque_get(frame->data, pop_ind), free_char_node, NULL);
  Deque_T col = pop(frame->data);
  Deque_free(&col);

  // 2. Add new column
  col = Deque_new();
  push(frame->data, col);

  // 3. add values based on row, col indices from data

  // TODO: move fd to correct line based on index instead of starting from beginning
  rewind(fd);

  int icol = 0;
  if (data->headers) {

    free(pop(frame->headers));

    // TODO: return error code : unable to load first line
    if (get_line(NULL, line, 255, fd) != E_OK) return 0; 

    while ((word = GET_TOK_R(line, delim, &saveptr)) && icol++ < new_col_ind) ;

    if (!word) return 0; // return error code
    push(frame->headers, strdup(word));
  }

  int irow = 0;
  while (get_line(NULL, line, 255, fd) == E_OK) {
    if (irow > data->inframe.last_row) break;
    else if (irow >= data->inframe.first_row) {
      saveptr = NULL;
      icol = 0;
      while ((word = GET_TOK_R(line, delim, &saveptr)))
        if (icol++ >= new_col_ind) break;

      // TODO: return error code here: value for selected column doesn't exist
      if (!word) return 0; 
      Deque_addhi(col, strdup(word));
    }
    irow++;
  }

  data->inframe.first_col += n;
  data->inframe.last_col += n;

  free(line);

  // TODO: return error status
  return 1;

}

int file_shift_row(Data_T data, Frame_T frame, int n, void *args) {

  FILE *fd = ((file_args) args)->fd;
  char delim = ((file_args) args)->delim;

  char *line = CALLOC(LINE_LEN, sizeof(char));
  char *word = NULL, *saveptr = NULL;
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int new_row_ind;

  if (n == 1) { // add row on bottom (scroll down)
    new_row_ind = data->inframe.last_row + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) { // add row on top (scroll up)
    new_row_ind = data->inframe.first_row - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  // TODO: return error code: invalid n
  } else return 0;

  // TODO: use row index
  rewind(fd);

  // Fast-forward to the correct row
  int irow = -data->headers;
  while (1) {
    if (get_line(NULL, line, 255, fd) == E_OK) {
      if (irow++ < new_row_ind) continue;
      else break;
      // TODO: add error code: no new row
    } else if (irow <= new_row_ind) return 0; 
  }

  int icol = 0, i = 0;
  while ((word = GET_TOK_R(line, delim, &saveptr))) {
    if (icol > data->inframe.last_col) break;
    if (icol >= data->inframe.first_col) {
      // TODO: add error code: value doesn't exist
      if (!word) return 0;
      Deque_T col = Deque_get(frame->data, i);
      free(pop(col));
      push(col, strdup(word));
      i++;
    }
    icol++;
  }

  data->inframe.first_row += n;
  data->inframe.last_row += n;
  
  return 1;

}

int file_close(void *args) {

  FILE *fd = ((file_args) args)->fd;

  // TODO: return error code: unable to close fd
  if (fclose(fd)) return 0;
  
  // TODO: return error code: all ok
  return 1;

}

Data_T Data_file_init(char *path, char delim, int headers) {

  Data_T data;
  NEW0(data);

  data->headers = headers ? 1 : 0; // any non-zero interpreted as 1
  data->open = file_open;
  data->load = file_load;
  data->shift_col = file_shift_col;
  data->shift_row = file_shift_row;;
  data->close = file_close;

  file_args args;
  NEW0(args);

  args->path = path;
  args->delim = delim;
  data->args = args;

  return data;

}
