// 
// -----------------------------------------------------------------------------
// argparse.h
// -----------------------------------------------------------------------------
//
// Argument parser for main program
//
// Tyler Wayne
// 

#include <argp.h>
#include <stdlib.h> // strtol

// TODO: do error checking on arguments here

const char *argp_program_version = "Preview v0.1\n"
  "Copyright © 2021 Tyler Wayne\n"
  "Licensed under the Apache License, Version 2.0\n"
  "\n"
  "Written by Tyler Wayne.";

const char *argp_program_bug_address = "<tylerwayne3@gmail.com>";

struct arguments {
  char *path;
  int col_width;
  char delim;
  int headers;
};

static struct argp_option options[] = {
  {"delimiter", 'd', "DELIM", 0, "Use DELIM instead of COMMA"},
  {"col-width", 'c', "NUM", 0, "Character width of columns"},
  {"no-header", 'h', 0, 0, "Enable header row"},
  {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {

  struct arguments *arguments = state->input;

  switch (key) {
    case 'd':
      arguments->delim = arg[0];
      break;

    case 'c':
      arguments->col_width = strtol(arg, NULL, 10);
      break;

    case 'h':
      arguments->headers = 1;
      break;

    // Position args
    case ARGP_KEY_ARG:
      // Too many arguments
      if (state->arg_num > 1) argp_usage(state);
      // arguments->args[state->arg_num] = arg;
      arguments->path = arg;
      break;

    case ARGP_KEY_END: 
      // Not enough arguments
      if (state->arg_num < 1) argp_usage(state); 
      break;

    default: 
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static char args_doc[] = "path";
static char doc[] = "preview -- display delimited data for quick investigation";
static struct argp argp = { options, parse_opt, args_doc, doc };
