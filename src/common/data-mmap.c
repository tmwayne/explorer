//
// -----------------------------------------------------------------------------
// data-mmap.c
// -----------------------------------------------------------------------------
//
// Implementation of Data_T instance to load data using mmap
//

#include <stdlib.h>   // exit, EXIT_FAILURE
#include <string.h>   // strdup

#include <sys/mman.h> // mmap, MAP_FAILED
#include <sys/stat.h> // fstat, open
#include <fcntl.h>    // O_RDONLY
#include <unistd.h>   // ssize_t, write, close, sysconf
#include "mem.h"      // NEW0, CALLOC, FREE

#include "deque.h"
#include "frame.h"
#include "errorcodes.h"


// TODO: set this dynamically?
#define MAX_ROWS 8192

#define TOK_OK    0
#define TOK_EOL   1
#define TOK_EOF   2
#define TOK_ERR   3

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

  char *ptr = ((mmap_args) args)->ptr;
  ssize_t st_size = ((mmap_args) args)->st_size;
  char delim = ((mmap_args) args)->delim;
  int *row_offsets = ((mmap_args) args)->row_offsets;

  Deque_T col = NULL;

  char *saveptr = NULL, *tok = NULL;
  int ret;

  int nbytes, total_bytes=0;
  int irow = 0, icol = 0;

  // Parse the first row to set up columns,
  // if headers then set those
  while (1) {
    ret = get_tok_r(&tok, &nbytes, ptr, delim, &saveptr, st_size);
    if (ret > 1) return E_DTA_PARSE_ERROR; // EOL
    total_bytes += nbytes;

    if (icol < frame->max_cols) {
      col = Deque_new();
      Deque_addhi(frame->data, col);
      frame->ncols++;
      if (data->headers) Deque_addhi(frame->headers, tok); 
      else Deque_addhi(col, tok);
    }

    data->ncols++;
    icol++;
    
    if (ret == TOK_EOL) {
      // Always start data at index 2
      if(data->headers) {
        row_offsets[1] = total_bytes;
        frame->nrows++;
      } else {
        row_offsets[2] = total_bytes;
        frame->nrows += 2;
        irow++;
      }
      break;
    }
  }

  irow++, icol = 0;
  while (irow < frame->max_rows) {
    ret = get_tok_r(&tok, &nbytes, ptr, delim, &saveptr, st_size);
    if (ret > 2) return E_DTA_PARSE_ERROR; // EOL, EOF are okay
    if (ret == TOK_EOF) break;
    total_bytes += nbytes;

    if (icol < frame->max_cols) {
      col = Deque_get(frame->data, icol);
      Deque_addhi(col, tok);
    }

    if (ret == TOK_EOL) {
      // TODO: how can we return row number of error?
      if (icol != data->ncols-1) return E_DTA_MISSING_FIELD;
      row_offsets[irow+1] = total_bytes;
      irow++;
      icol = 0;
      frame->nrows++;
    } else icol++;
  }

  // Assume there is a always a header, for consistent indexing
  data->inframe.first_row = 1; 
  data->inframe.last_col = frame->ncols - 1;
  data->inframe.last_row = frame->nrows - 1;
  data->nrows = frame->nrows; 

  return E_OK;

}

static int shift_col(Data_T data, Frame_T frame, int n, void *args) {

  // TODO: parse values and put in temporary array before modifying frame

  char *ptr = ((mmap_args) args)->ptr;
  char delim = ((mmap_args) args)->delim;
  int *row_offsets = ((mmap_args) args)->row_offsets;
  
  void *(*pop)(Deque_T deque);
  void *(*push)(Deque_T deque, void *x);
  int new_col_ind;

  if (n == 1) { // add col to the right
    new_col_ind = data->inframe.last_col + 1;
    pop = Deque_remlo;
    push = Deque_addhi;
  } else if (n == -1) {      // add col to the left
    new_col_ind = data->inframe.first_col - 1;
    pop = Deque_remhi;
    push = Deque_addlo;
  } else return E_DTA_INVALID_INPUT;

  if (new_col_ind >= data->ncols) return E_DTA_COL_OOB;

    // 1. Free values in first column
  Deque_T col = pop(frame->data);
  Deque_free(&col);

  // 2. Add new column
  col = Deque_new();
  push(frame->data, col);

  // 3. add values based on row, col indices from data

  int icol = 0, irow, ret, nbytes;
  char *tok = NULL, *saveptr = NULL;
  int len = row_offsets[1]; // length of header row
  char *start = ptr;

  if (data->headers) {
    pop(frame->headers);

    while (1) {
      ret = get_tok_r(&tok, &nbytes, start, delim, &saveptr, len);
      if (ret != TOK_OK) return E_DTA_PARSE_ERROR;
      if (icol++ == new_col_ind) {
        push(frame->headers, tok);
        break;
      }
    }
  }
  
  // TODO: write a note somewhere that we always assume headers
  icol = 0;
  saveptr = NULL;
  irow = data->inframe.first_row;
  start = ptr + row_offsets[irow];
  len = row_offsets[irow+1] - row_offsets[irow]; 
 
  while (1) {
    if (irow > data->inframe.last_row) break;
    ret = get_tok_r(&tok, &nbytes, start, delim, &saveptr, len);
    if (ret != TOK_OK) return E_DTA_PARSE_ERROR;
    if (icol == new_col_ind) {
      Deque_addhi(col, tok);
      icol = 0, irow++;
      saveptr = NULL;
      start = ptr + row_offsets[irow];
      len = row_offsets[irow+1] - row_offsets[irow];
      continue;
    }
    icol++;
  }

  data->inframe.first_col += n;
  data->inframe.last_col += n;

  return E_OK;

}

static int shift_row(Data_T data, Frame_T frame, int n, void *args) {

  char *ptr = ((mmap_args) args)->ptr;
  char delim = ((mmap_args) args)->delim;
  int *row_offsets = ((mmap_args) args)->row_offsets;
  ssize_t st_size = ((mmap_args) args)->st_size;

  char *tok = NULL, *saveptr = NULL;
  
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
  } else return E_DTA_INVALID_INPUT;

  // Fast-forward to the correct row
  int icol = 0, i = 0, ret;
  int nbytes, total_bytes = row_offsets[data->nrows];
  char *start = ptr + row_offsets[new_row_ind]; 

  // TODO: check that offset is available
  
  int len = st_size - row_offsets[new_row_ind];
  if (len <= 0) return E_DTA_ROW_OOB;

  while (1) {
    ret = get_tok_r(&tok, &nbytes, start, delim, &saveptr, len);
    total_bytes += nbytes;
    if (ret > 1) return E_DTA_PARSE_ERROR;
    if (icol >= data->inframe.first_col && icol <= data->inframe.last_col) {
      Deque_T col = Deque_get(frame->data, i);
      pop(col);
      push(col, tok);
      i++;
    } 
    if (icol == data->ncols-1) break;
    icol++;
  }

  if (new_row_ind+1 > data->nrows) {
    row_offsets[new_row_ind+1] = total_bytes;
    data->nrows++;
  }

  data->inframe.first_row += n;
  data->inframe.last_row += n;
  
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
