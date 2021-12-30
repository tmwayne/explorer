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

void Frame_free(Frame_T *frame) {
  
  // TODO: free frame->data
  // TODO: free frame->headers if exists
  // TODO: free frame
  // TODO: set frame to NULL
  
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

