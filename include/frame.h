// 
// -----------------------------------------------------------------------------
// frame.h
// -----------------------------------------------------------------------------
//
// Data structure for the visible frame shown by ncurses.
//
// Copyright © 2021 Tyler Wayne
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
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
