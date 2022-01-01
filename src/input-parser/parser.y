//
// -----------------------------------------------------------------------------
// parser.y
// -----------------------------------------------------------------------------
//
// Parser for input-parser
//
// Tyler Wayne © 2021
//

%{
#include <ncurses.h>
#include "preview.h"
#include "errorcodes.h"
%}

%union {
  char c;
  // char *s;
  // long i;
  // double f;
}

%token <c> LEFT RIGHT UP DOWN OTHER

// TODO: add error handling

%%

list:
  | list cmd
  | list OTHER               { return 0; }
  ;

cmd:
  LEFT                  {
                          if (frame->cursor.col/frame->col_width > 0) {
                            frame->cursor.col -= frame->col_width;
                            Frame_print(frame, O_FRM_CURS);
                          } else if (frame->data_loaded.first_col > 0) {
                            Frame_shift_col(frame, data, -1);
                            Frame_print(frame, O_FRM_DATA);
                          }
                        }
  | RIGHT               {
                          if (frame->cursor.col/frame->col_width < frame->ncols-1) {
                            frame->cursor.col += frame->col_width;
                            Frame_print(frame, O_FRM_CURS);
                          } else if (frame->data_loaded.last_col < data->ncols-1) {
                            Frame_shift_col(frame, data, 1);
                            Frame_print(frame, O_FRM_DATA);
                          }
                          // TODO: print errors (parse/oob) in status row
                        }
  | UP                  {
                          if (frame->cursor.row > 0) {
                            frame->cursor.row--;
                            Frame_print(frame, O_FRM_CURS);
                          } else if 
                            (frame->data_loaded.first_row > !!frame->headers) {
                            Frame_shift_row(frame, data, -1);
                            Frame_print(frame, O_FRM_DATA);
                          }
                        }

  | DOWN                {
                          if (frame->cursor.row < frame->nrows - 1) {
                            frame->cursor.row++;
                            Frame_print(frame, O_FRM_CURS);
                          } else if (Frame_shift_row(frame, data, 1) == E_OK)
                            Frame_print(frame, O_FRM_DATA);
                          // TODO: print errors (parse/oob) in status row
                        }
  ;

%%
