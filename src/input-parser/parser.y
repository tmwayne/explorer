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

#define UNHIGHLIGHT() chgat(frame->col_width-1, A_NORMAL, 0, NULL)
#define MOVE_HIGHLIGHT_REFRESH() do { \
  move(frame->cursor.row, frame->cursor.col); \
  chgat(frame->col_width-1, A_REVERSE, 0, NULL); \
  refresh(); \
  } while (0)

%}

%union {
  char c;
  // char *s;
  // long i;
  // double f;
}

%token <c> LEFT RIGHT UP DOWN OTHER

%%

list:
  | list cmd
  | OTHER               { done = 0; return 0; }
  ;

cmd:
  LEFT                  {
                          UNHIGHLIGHT();
                          if (frame->cursor.col/frame->col_width > 0)
                            frame->cursor.col -= frame->col_width;
                          else if (data->inframe.first_col > 0) {
                            data->shift_col(data, frame, -1, data->args);
                            Frame_print(frame);
                          }
                          MOVE_HIGHLIGHT_REFRESH();
                        }
  | RIGHT               {
                          UNHIGHLIGHT();
                          if (frame->cursor.col/frame->col_width < frame->ncols-1)
                            frame->cursor.col += frame->col_width;
                          else if (data->inframe.last_col < data->ncols-1) {
                            data->shift_col(data, frame, 1, data->args);
                            Frame_print(frame);
                          }
                          MOVE_HIGHLIGHT_REFRESH();
                        }
  | UP                  {
                          UNHIGHLIGHT();
                          if (frame->cursor.row > 0) 
                            frame->cursor.row--;
                          else if (data->inframe.first_row > 0) {
                            data->shift_row(data, frame, -1, data->args);
                            Frame_print(frame);
                          }
                          MOVE_HIGHLIGHT_REFRESH();
                        }

  | DOWN                {
                          UNHIGHLIGHT();
                          if (frame->cursor.row < frame->nrows + data->headers - 1) 
                            frame->cursor.row++;
                          else if (data->shift_row(data, frame, 1, data->args))
                            Frame_print(frame);
                          MOVE_HIGHLIGHT_REFRESH();
                        }
  ;

%%
