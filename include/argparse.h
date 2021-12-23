// 
// -----------------------------------------------------------------------------
// argparse.h
// -----------------------------------------------------------------------------
//
// Argument parser for main program
//

#include <argp.h>
#include <dict.h> // dict_T, dict_set

const char *argp_program_version = "Program version\n";
const char *argp_program_bug_address = "<tylerwayne3@gmail.com>";

static struct argp_option options[] = {
  // {"with-value", 'a', "file", 0, "Optional argument with value"},
  // {"without-value", 'b', 0, 0, "Optional argument without value"},
  {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {

  dict_T configs = state->input;

  switch (key) {
    // case 'a': arguments->with = arg;  break;
    // case 'b': arguments->without = 1; break;

    // stop at the first positional arg and capture the remaining ones
    case ARGP_KEY_ARG:
      dict_set(configs, "exercise", arg);
      dict_set(configs, "argv", &state->argv[--state->next]);
      dict_set(configs, "argc", (void *) (long) state->argc - state->next);
      state->next = state->argc; // stop parsing
      break;

    // Here we can enforce the number of positional arguments if we so choose
    // case ARGP_KEY_END: if (state->arg_num < 1) argp_usage(state); break;
    default: return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static char args_doc[] = "exercise";
static char doc[] = "gym -- train with various exercises and record results";
static struct argp argp = { options, parse_opt, args_doc, doc };
