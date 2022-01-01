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

// TODO: change these to powers of two to be able to test for multiples
#define TOK_OK    0
#define TOK_EOL   1
#define TOK_EOF   2
#define TOK_ERR   3

typedef struct mmap_args {
  char *path;
  char delim;
  char *ptr;
  ssize_t st_size;
  int *row_offsets;
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

  close(fd);

  if (ptr == MAP_FAILED) return E_DTA_FILE_ERROR;

  _args->st_size = statbuf.st_size;
  _args->ptr = ptr;

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

  // TODO: check that strlen(path)>0
  // TODO: check that delim is not NULL

  Data_T data;
  NEW0(data);

  // TODO: do we need headers?
  data->headers = headers ? 1 : 0; // any non-zero interpreted as 1
  data->open = data_open;
  data->get_col = get_col;
  data->get_row = get_row;;
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
