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
                          } else if (data->inframe.first_col > 0) {
                            Data_shift_col(data, frame, -1);
                            Frame_print(frame, O_FRM_DATA);
                          }
                        }
  | RIGHT               {
                          if (frame->cursor.col/frame->col_width < frame->ncols-1) {
                            frame->cursor.col += frame->col_width;
                            Frame_print(frame, O_FRM_CURS);
                          } else if (data->inframe.last_col < data->ncols-1) {
                            Data_shift_col(data, frame, 1);
                            Frame_print(frame, O_FRM_DATA);
                          }
                          // TODO: print errors (parse/oob) in status row
                        }
  | UP                  {
                          if (frame->cursor.row > 0) {
                            frame->cursor.row--;
                            Frame_print(frame, O_FRM_CURS);
                          } else if (data->inframe.first_row > 1) {
                            Data_shift_row(data, frame, -1);
                            Frame_print(frame, O_FRM_DATA);
                          }
                        }

  | DOWN                {
                          if (frame->cursor.row < frame->nrows - 1) {
                            frame->cursor.row++;
                            Frame_print(frame, O_FRM_CURS);
                          } else if (Data_shift_row(data, frame, 1) == E_OK)
                            Frame_print(frame, O_FRM_DATA);
                          // TODO: print errors (parse/oob) in status row
                        }
  ;

%%
