//
// -----------------------------------------------------------------------------
// preview.c
// -----------------------------------------------------------------------------
//
// Development version of Preview
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

#include <stdio.h>    // fprintf
#include <stdlib.h>   // exit, EXIT_FAILURE
#include <ncurses.h>
#include "argparse.h" // arguments, argp_parse
#include "preview.h"
#include "errorcodes.h"

#define EXIT(msg) do { \
  endwin(); \
  fprintf(stderr, "%s\n", (msg)); \
  exit(EXIT_FAILURE); \
  } while (0)

void print_char_node(void **x, void *cl) {

  printf("%s\n", * (char **) x);

}

void yyerror(char *s) {

  fprintf(stderr, "%s\n", s);

}

// TODO: make parser reentrant to avoid global variables
Frame_T frame;
Data_T data;

int main(int argc, char **argv) {

  // TODO: setup configparse

  // Default arguments
  struct arguments arguments;
  arguments.headers = 0;
  arguments.col_width = 16;
  arguments.delim = ',';

  // Command line arguments
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
 
  initscr();
  cbreak();    // disable line buffering
  noecho();    // disable echo for getch
  curs_set(0); // hide cursor

  int max_rows, max_cols;
  getmaxyx(stdscr, max_rows, max_cols);
  max_cols /= arguments.col_width;

  // TODO: determine appropriate column width
  frame = Frame_init(
    arguments.col_width,  // col_width
    max_cols,             // max_cols
    max_rows-1,           // max_rows
    arguments.headers     // headers
  );
  if (!frame) EXIT("Error initializing frame\n");

  data = Data_mmap_init(
    arguments.path,       // path
    arguments.delim       // delim
  );
  if (!data) EXIT("Error initializing data\n");

  int err = Data_open(data);
  if (err) EXIT("Error opening data\n");

  err = Frame_load(frame, data);
  if (err) {
    endwin();
    switch (err) {
      case E_DTA_PARSE_ERROR:
        fprintf(stderr, "Error parsing file\n");
        break;
      case E_DTA_MISSING_FIELD:
        fprintf(stderr, "Row has incorrect number of fields\n");
        break;
      default:
        fprintf(stderr, "Error loading data\n");
    }
    exit(EXIT_FAILURE);
  }

  Frame_print(frame, data, O_FRM_DATA | O_FRM_CURS);

  err = yyparse();
  if (err) EXIT("Error parsing user input\n");

  endwin();

  if (Data_close(data)) {
    fprintf(stderr, "Error closing data\n");
    exit(EXIT_FAILURE);
  }

  Frame_free(&frame, data->free_node, NULL);
  Data_mmap_free(&data);

}
