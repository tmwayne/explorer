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

#define Data_open(data) (data->open)(data)
#define Data_close(data) (data->close)(data)

// TODO: set this dynamically?
#define MAX_ROWS 8192
#define MAX_COLS 1024

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
  struct data_loaded {
    int first_row;
    int first_col;
    int last_row;
    int last_col;
  } data_loaded;
  Deque_T headers;
  Deque_T data;
} *Frame_T;

typedef struct Data_T {
  char *path;
  char delim;
  int *row_offsets;
  ssize_t st_size;
  int ncols;
  int nrows;
  int (*open)(struct Data_T *data);

  int (*get_col)(struct Data_T *data, char **buf, 
    int col, int row_start, int row_end);

  int (*get_row)(struct Data_T *data, char **buf,
    int row, int col_start, int col_end);

  int (*mvaddntok)(int row, int col, const char *str,
    int n, char delim);

  int (*close)(struct Data_T *data);
  void (*free_node)(void **node, void *args);
  void *args;
} *Data_T;

extern Frame_T  Frame_init(int col_width, int max_cols, int max_rows, int headers);
extern int      Frame_load(Frame_T frame, Data_T data);
extern void     Frame_free(Frame_T *frame, 
                  void free_node(void **node, void *args), void *args);
extern int      Frame_shift_row(Frame_T frame, Data_T data, int n);
extern int      Frame_shift_col(Frame_T frame, Data_T data, int n);
extern int      Frame_print(Frame_T frame, Data_T data, int action);

extern Data_T Data_mmap_init(char *path, char delim);
extern void   Data_mmap_free(Data_T *data);

#endif
