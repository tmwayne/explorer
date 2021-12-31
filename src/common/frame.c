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

