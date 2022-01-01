//
// -----------------------------------------------------------------------------
// data-mmap.c
// -----------------------------------------------------------------------------
//
// Implementation of Data_T instance to load data using mmap
//

#include <stdlib.h>   // exit, EXIT_FAILURE
#include <string.h>   // strdup
#include <stdint.h>   // uint8_t

#include <sys/mman.h> // mmap, MAP_FAILED
#include <sys/stat.h> // fstat, open
#include <fcntl.h>    // O_RDONLY
#include <unistd.h>   // ssize_t, write, close, sysconf
#include "mem.h"      // NEW0, CALLOC, FREE

#include "deque.h"
#include "frame.h"
#include "errorcodes.h"

// TODO: write a note somewhere that we always assume headers

// TODO: set this dynamically?
#define MAX_ROWS 8192
#define MAX_COLS 1024

// TODO: change these to powers of two to be able to test for multiples
#define TOK_OK    0
#define TOK_EOL   1
#define TOK_EOF   2
#define TOK_ERR   3

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct mmap_args {
  char *path;
  char delim;
  char *ptr;
  ssize_t st_size;
  int *row_offsets; // Currently handles up to ~4GB
} *mmap_args;

static int get_tok_r(char **tok, int *nbytes, char *str, const char delim, 
  char **saveptr, ssize_t len) {

  // TODO: check for trailing whitespace in file

  if (*saveptr == NULL) *saveptr = str;

  char *field = *saveptr;
  // TODO: make this a parameter
  int in_quote = 0;

  *nbytes = 1;
  for ( int i=0; ; i++, (*nbytes)++) {

    switch(field[i]) {
      case '\0':
        if (*saveptr+i >= str+len) return TOK_EOF;
        *saveptr += (i+1);
        *tok = field;
        return TOK_OK;

      case '\n':
        field[i] = '\0';
        *saveptr += (i+1);
        *tok = field;
        return TOK_EOL;

      case '"':
        in_quote = in_quote ? 0 : 1;
        break;

      default:
        if (field[i]==delim && !in_quote) {
          field[i] = '\0';
          *saveptr += (i+1);
          *tok = field;
          return TOK_OK;
        }
    }
  }

  return TOK_ERR;

}

static int get_row(Data_T data, char **buf, int row, int col_start, int col_end) {

  // TODO: currently only supports one row lookahead
  // TODO: increase data->nrows if necessary

  int parsed = 0;
  int *row_offsets = ((mmap_args) data->args)->row_offsets;
  ssize_t len = ((mmap_args) data->args)->st_size;
  char *ptr = ((mmap_args) data->args)->ptr;
  char delim = ((mmap_args) data->args)->delim;
  int err, nbytes, total_bytes;
  char *tok, *saveptr = NULL;

  int i = 0, icol = 0; // indexing values

  // Input checks
  if (data->ncols && col_end >= data->ncols) return E_DTA_COL_OOB;
  if (data->ncols && col_end == -1) col_end = data->ncols-1;

  if (row > 0 && (row != 1 || !data->headers)) {
    if (row_offsets[row] == len) return E_DTA_EOF;
    if (!row_offsets[row]) return E_DTA_ROW_OOB; 
  }

  if (row_offsets[row+1]) {
    len = row_offsets[row+1] - row_offsets[row];
    parsed = 1;
  } 

  if (!data->ncols && parsed) return E_DTA_BAD_INPUT;

  if (parsed) {

    while (1) {
      err = get_tok_r(&tok, &nbytes, ptr+row_offsets[row], delim, &saveptr, len);
      if (err == TOK_ERR || err == TOK_EOF) 
        return E_DTA_PARSE_ERROR; // TODO: check this
      if (icol >= col_start && icol <= col_end) buf[i++] = tok;
      if (icol == col_end) break;
      icol++;
    }

  } else {

    // TODO: this assumes headers

    total_bytes = row_offsets[row];
    while (1) {
      err = get_tok_r(&tok, &nbytes, ptr+row_offsets[row], delim, &saveptr, len);
      if (err == TOK_ERR) return E_DTA_PARSE_ERROR; // EOL, EOF are okay
      else if (err == TOK_EOF) break; 

      if (icol >= col_start && (icol <= col_end || col_end == -1)) 
        buf[i++] = tok;
      total_bytes += nbytes;

      if (err == TOK_EOL) {
        if (!data->ncols) data->ncols = icol+1;
        if (icol != data->ncols-1) return E_DTA_MISSING_FIELD;
        else break;
      }

      icol++;
    }
    // row_offsets[row + 1] = total_bytes;
    if (row == 0 && !data->headers) row_offsets[row + 2] = total_bytes;
    else row_offsets[row + 1] = total_bytes;
    // row_offsets[row + !data->headers + 1] = total_bytes;
  }

  return E_OK;

}

static int get_col(Data_T data, char **buf, int col, int row_start, int row_end) {

  int *row_offsets = ((mmap_args) data->args)->row_offsets;
  char *ptr = ((mmap_args) data->args)->ptr;
  char delim = ((mmap_args) data->args)->delim;
  int err, nbytes; // , total_bytes;
  char *tok = NULL, *saveptr;

  if (col > data->ncols-1) return E_DTA_COL_OOB;

  // TODO: check that row_end isn't past data->nrows
  // TODO: currently doesn't check validity of rows / row_offsets
  
  for (int irow=row_start, i=0; irow<=row_end; irow++, i++) {
    saveptr = NULL;
    int len = row_offsets[irow+1] - row_offsets[irow];
    for (int icol=0; icol <= col; icol++) {
      err = get_tok_r(&tok, &nbytes, ptr+row_offsets[irow], delim, &saveptr, len);
      if (err == TOK_ERR || err == TOK_EOF)
        return E_DTA_PARSE_ERROR;
    }
    buf[i] = tok;
  }

  return E_OK;

}

static int data_open(void *args) {

  mmap_args _args = args;

  int fd = open(_args->path, O_RDONLY);
  if (fd < 0) return E_DTA_FILE_ERROR;

  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0) return E_DTA_FILE_ERROR;

  long PAGESIZE = sysconf(_SC_PAGESIZE);
  if (statbuf.st_size % PAGESIZE == 0) return E_DTA_FILE_ERROR;

  char *ptr = mmap(
    NULL,                   // address
    statbuf.st_size,        // length
    PROT_READ | PROT_WRITE, // protection level
    MAP_PRIVATE,            // flags
    fd,                     // file descriptor
    0                       // offset
  );

  // TODO: how should this interplay with the next check?
  close(fd);

  if (ptr == MAP_FAILED) return E_DTA_FILE_ERROR;

  _args->st_size = statbuf.st_size;
  _args->ptr = ptr;

  return E_OK;

}

static int data_load(Data_T data, Frame_T frame, void *args) {

  Deque_T col = NULL;
  int ret, irow = 0, icol = 0;
  char *buf[MAX_COLS] = { 0 };

  // TODO: return appropriate error code
  // Load first row, which may be header
  if ((ret = get_row(data, buf, 0, 0, frame->max_cols-1)) != E_OK) 
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

    ret = get_row(data, buf, irow, 0, frame->ncols-1);
    if (ret == E_DTA_EOF) break;
    else if (ret != E_OK) return E_DTA_PARSE_ERROR;

    for (icol = 0; icol < frame->ncols; icol++) {
      col = Deque_get(frame->data, icol);
      Deque_addhi(col, buf[icol]);
    }

    frame->nrows++;
  }

  // Assume there is a always a header, for consistent indexing
  data->inframe.first_row = 1; 
  data->inframe.last_col = frame->ncols - 1;
  data->inframe.last_row = frame->nrows - 1;
  data->nrows = frame->nrows; 

  return E_OK;

}
static int shift_row(Data_T data, Frame_T frame, int n, void *args) {
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int new_row_ind;

  if (n == 1) { // add row on bottom (scroll down)
    new_row_ind = data->inframe.last_row + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) { // add row on top (scroll up)
    new_row_ind = data->inframe.first_row - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  } else return E_DTA_BAD_INPUT;

  if (new_row_ind+1 == MAX_ROWS) return E_DTA_MAX_ROWS;

  char *buf[frame->ncols];

  int ret = get_row(data, buf, new_row_ind, 0, frame->ncols-1);
  if (ret == E_DTA_EOF) return E_DTA_EOF;
  if (ret != E_OK) return E_DTA_PARSE_ERROR;

  int icol = data->inframe.first_col, i = 0;
  while (icol <= data->inframe.last_col) {
    Deque_T col = Deque_get(frame->data, i);
    pop(col);
    push(col, buf[i]);
    icol++, i++;
  }

  data->inframe.first_row += n;
  data->inframe.last_row += n;
  
  return E_OK;

}

static int shift_col(Data_T data, Frame_T frame, int n, void *args) {
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int new_col_ind;

  if (n == 1) {           // add col to the right
    new_col_ind = data->inframe.last_col + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) {   // add col to the left
    new_col_ind = data->inframe.first_col - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  } else return E_DTA_BAD_INPUT;

  if (new_col_ind >= data->ncols) return E_DTA_COL_OOB;

  // Get new values from data
  char *header_buf = NULL;
  char *data_buf[frame->nrows];

  // Load data
  if (data->headers) {
    if (get_col(data, &header_buf, new_col_ind, 0, 0) != E_OK)
      return E_DTA_PARSE_ERROR;
  }

  int ret = get_col(data, data_buf, new_col_ind, 
    data->inframe.first_row, data->inframe.last_row);

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

  data->inframe.first_col += n;
  data->inframe.last_col += n;

  return E_OK;

}

static int data_close(void *args) {

  char *ptr = ((mmap_args) args)->ptr;
  ssize_t st_size = ((mmap_args) args)->st_size;

  if (munmap(ptr, st_size) != 0)
    return E_DTA_RESOURCE_ERROR;
  
  return E_OK;

}

Data_T Data_mmap_init(char *path, char delim, int headers) {

  // TODO: error check argument values

  Data_T data;
  NEW0(data);

  data->headers = headers ? 1 : 0; // any non-zero interpreted as 1
  data->open = data_open;
  data->load = data_load;
  data->shift_col = shift_col;
  data->shift_row = shift_row;;
  data->close = data_close;

  mmap_args args;
  NEW0(args);

  args->path = path;
  args->delim = delim;
  args->row_offsets = calloc(MAX_ROWS, sizeof(int));

  data->args = args;

  // TODO: if issues, return NULL
  return data;

}

void Data_mmap_free(Data_T *data) {

  assert(data && *data && (*data)->args); 

  mmap_args args = (*data)->args;
  assert(args->row_offsets);

  FREE(args->row_offsets);
  FREE(args);
  FREE(*data);

}
