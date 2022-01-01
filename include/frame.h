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
#include "deque.h" // Deque_T

#define O_FRM_CURS 1
#define O_FRM_DATA 2

#define Data_shift_col(data, frame, n) \
  (data)->shift_col((data), (frame), (n), (data)->args)

#define Data_shift_row(data, frame, n) \
  (data)->shift_row((data), (frame), (n), (data)->args)

typedef struct Frame_T {
  int col_width;
  int max_cols;
  int max_rows;
  int ncols;
  int nrows;
  struct cursor {
    int row;
    int col;
  } cursor;
  Deque_T headers;
  Deque_T data;
} *Frame_T;

typedef struct Data_T {
  int ncols;
  int nrows;
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
extern void     Frame_free(Frame_T *frame);
extern int      Frame_print(Frame_T frame, unsigned char what);

extern Data_T Data_file_init(char *path, char delim, int headers);
extern void   Data_file_free(Data_T *data);

extern Data_T Data_mmap_init(char *path, char delim, int headers);
extern void   Data_mmap_free(Data_T *data);

#endif
