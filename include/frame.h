// 
// -----------------------------------------------------------------------------
// frame.h
// -----------------------------------------------------------------------------
//
// Data structure for the visible frame shown by ncurses.
//
// Tyler Wayne © 2021
//

#ifndef FRAME_INCLUDED
#define FRAME_INCLUDED

#include <ncurses.h>

struct cursor {
  int row;
  int col;
};

typedef struct Frame_T {
  int col_width;
  int max_cols;
  int max_rows;
  int ncols;
  int nrows;
  struct cursor cursor;
  Deque_T headers;
  Deque_T data;
} *Frame_T;

typedef struct Data_T {
  int ncols;
  int headers;
  struct inframe {
    int first_row;
    int first_col;
    int last_row;
    int last_col;
  } inframe;
  int (*open)(void *args);
  int (*load)(struct Data_T *data, Frame_T frame, void *args);
  int (*shift_col)(struct Data_T *data, Frame_T frame, int n, void *args);
  int (*shift_row)(struct Data_T *data, Frame_T frame, int n, void *args);
  int (*close)();
  void *args;
} *Data_T;

extern Frame_T  Frame_init(int col_width, int max_cols, int max_rows, int headers);
extern int      Frame_print(Frame_T frame);

extern Data_T Data_file_init(char *path, int headers);

#endif
