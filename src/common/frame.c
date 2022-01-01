//
// -----------------------------------------------------------------------------
// frame.c
// -----------------------------------------------------------------------------
//

#include <string.h>   // strdup
#include "mem.h"      // NEW0, CALLOC, FREE
#include "cstrings.h" // get_line, get_tok_r
#include "deque.h"
#include "frame.h"
#include "errorcodes.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void free_deque_node(void **x, void *cl) {
  
  Deque_free((Deque_T *) x);

}

Frame_T Frame_init(int col_width, int max_cols, int max_rows, int headers) {

  Frame_T frame;
  NEW0(frame);

  // TODO: need to put headers and data with the data to be
  // able to properly free it
  
  // TODO: add error checking to these values

  frame->col_width = col_width;
  frame->max_cols = max_cols;
  frame->max_rows = max_rows - (headers ? 1 : 0);
  frame->headers = headers ? Deque_new() : NULL;
  frame->data = Deque_new();

  return frame;

}

void Frame_free(Frame_T *frame) {

  assert(frame && *frame && (*frame)->data);

  if ((*frame)->headers) Deque_free(&(*frame)->headers);

  if (Deque_length((*frame)->data) > 0)
    Deque_map((*frame)->data, free_deque_node, NULL);

  Deque_free(&(*frame)->data);

  FREE(*frame);
  
}

int Frame_load(Frame_T frame, Data_T data) {

  Deque_T col = NULL;
  int ret, irow = 0, icol = 0;
  char *buf[MAX_COLS] = { 0 };

  // TODO: return appropriate error code
  // Load first row, which may be header
  if ((ret = data->get_row(data, buf, 0, 0, frame->max_cols-1)) != E_OK) 
    return E_DTA_PARSE_ERROR;

  frame->ncols = MIN(data->ncols, frame->max_cols);

  for (icol = 0; icol<frame->ncols; icol++) {
    col = Deque_new();
    Deque_addhi(frame->data, col);
    if (data->headers) Deque_addhi(frame->headers, buf[icol]);
    else Deque_addhi(col, buf[icol]);
  }

  if(data->headers) frame->nrows++;
  else {
    frame->nrows += 2;
    irow++;
  }

  // Load remaining rows
  for (irow++ ; irow < frame->max_rows ; irow++) {

    ret = data->get_row(data, buf, irow, 0, frame->ncols-1);
    if (ret == E_DTA_EOF) break;
    else if (ret != E_OK) return E_DTA_PARSE_ERROR;

    for (icol = 0; icol < frame->ncols; icol++) {
      col = Deque_get(frame->data, icol);
      Deque_addhi(col, buf[icol]);
    }

    frame->nrows++;
  }

  // Assume there is a always a header, for consistent indexing
  frame->data_loaded.first_row = 1;
  frame->data_loaded.last_col = frame->ncols - 1;
  frame->data_loaded.last_row = frame->nrows - 1;
  data->nrows = frame->nrows; 

  return E_OK;

}

int Frame_print(Frame_T frame, unsigned char what) {

  assert(frame && Deque_length(frame->data) > 0);
  
  // TODO: handle this error with error code or in data_load
  assert(!frame->headers || 
    Deque_length(frame->headers) == Deque_length(frame->data));

  // TODO: check somehow to see what has changed
  
  if (what & O_FRM_DATA) {

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

      // TODO: throw error if we can't get data

      for (int irow=0, n=0; irow<frame->nrows-1; irow++, n++) {
        mvaddnstr(irow + headers, text_start, Deque_get(col, irow), text_width);
        if (icol < frame->ncols-1) // print for all but the last column
          mvaddstr(irow + headers, (icol+1)*frame->col_width-1, "|");
      }
    }

  } 

  if (what & O_FRM_CURS) {
    chgat(frame->col_width-1, A_NORMAL, 0, NULL);
  }

  move(frame->cursor.row, frame->cursor.col);
  chgat(frame->col_width-1, A_REVERSE, 0, NULL);
  refresh();

  return E_OK;

}

int Frame_shift_row(Frame_T frame, Data_T data, int n) {
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int new_row_ind;

  if (n == 1) { // add row on bottom (scroll down)
    new_row_ind = frame->data_loaded.last_row + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) { // add row on top (scroll up)
    new_row_ind = frame->data_loaded.first_row - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  } else return E_DTA_BAD_INPUT;

  if (new_row_ind+1 == MAX_ROWS) return E_DTA_MAX_ROWS;

  char *buf[frame->ncols];

  int ret = data->get_row(data, buf, new_row_ind, 0, frame->ncols-1);
  if (ret == E_DTA_EOF) return E_DTA_EOF;
  if (ret != E_OK) return E_DTA_PARSE_ERROR;

  int icol = frame->data_loaded.first_col, i = 0;
  while (icol <= frame->data_loaded.last_col) {
    Deque_T col = Deque_get(frame->data, i);
    pop(col);
    push(col, buf[i]);
    icol++, i++;
  }

  frame->data_loaded.first_row += n;
  frame->data_loaded.last_row += n;
  
  return E_OK;

}

int Frame_shift_col(Frame_T frame, Data_T data, int n) {
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int new_col_ind;

  if (n == 1) {           // add col to the right
    new_col_ind = frame->data_loaded.last_col + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) {   // add col to the left
    new_col_ind = frame->data_loaded.first_col - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  } else return E_DTA_BAD_INPUT;

  if (new_col_ind >= data->ncols) return E_DTA_COL_OOB;

  // Get new values from data
  char *header_buf = NULL;
  char *data_buf[frame->nrows];

  // Load data
  if (data->headers) {
    if (data->get_col(data, &header_buf, new_col_ind, 0, 0) != E_OK)
      return E_DTA_PARSE_ERROR;
  }

  int ret = data->get_col(data, data_buf, new_col_ind, 
    frame->data_loaded.first_row, frame->data_loaded.last_row);

  if (ret != E_OK) return E_DTA_PARSE_ERROR;

  // Update frame
  if (data->headers) {
    pop(frame->headers);
    push(frame->headers, header_buf);
  }

  Deque_T col = pop(frame->data);
  Deque_free(&col);

  col = Deque_new();
  push(frame->data, col);

  for (int i = 0; i<frame->nrows-1; i++)
    Deque_addhi(col, data_buf[i]);

  frame->data_loaded.first_col += n;
  frame->data_loaded.last_col += n;

  return E_OK;

}

