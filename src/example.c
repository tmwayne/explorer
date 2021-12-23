//
// -----------------------------------------------------------------------------
// main.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdlib.h>      // calloc
#include <time.h>        // time_t,
#include "argparse.h"    // argp_parse

#include <dict.h>        // dict_T, dict_new, dict_free
#include <configparse.h> // configparse
#include "registry.h"    // load_plugins
#include "exercise.h"    // exercise_T, exercise_init

#define DEFAULT_USER_RC_PATH "/home/tyler/.config/brainasiumrc"
#define DEFAULT_EXERCISE_DIR "/home/tyler/.local/lib/brainasium/exercises"
#define DEFAULT_EXERCISE "sample"

static void set_defaults(dict_T);
static char *timestamp(char *, size_t);

int main(int argc, char** argv) {

  // Set defaults
  dict_T configs = dict_new();
  set_defaults(configs);

  // Load configurations
  FILE *userrc = fopen(DEFAULT_USER_RC_PATH, "r");
  if (userrc) configparse(configs, userrc);

  // Load command-line arguments

  // ARGP_IN_ORDER : force arguments to be parsed in order 
  // so that flags aren't parsed early
  argp_parse(&argp, argc, argv, ARGP_IN_ORDER, 0, configs);
  // TODO: free any structures related to argp

  // Remaining arguments are passed to the exercise program
  argc = (int) (long) dict_get(configs, "argc");
  argv = dict_get(configs, "argv");

  // Load exercise
  dict_T registry = dict_new();
  load_plugins(registry, dict_get(configs, "exercise_dir"));

  char *selection = dict_get(configs, "exercise");
  if (!dict_get(registry, selection)) {
    fprintf(stderr, "Exercise selection not recognized...\n");
    exit(EXIT_FAILURE);
  }
  exercise_T exercise = exercise_init(registry, selection);
  dict_free(&registry, (void (*)(void *)) entry_free);

  // Play
  time_t start = time(NULL);
  double score = exercise->play(argc, argv);

  time_t elapsed = time(NULL) - start;
  printf("It took you %ld seconds...\n", elapsed);

  // Save output
  FILE *fout = fopen(dict_get(configs, "output_file"), "a");
  if (fout) {
    char now[20];
    fprintf(fout, "%s|", timestamp(now, 20));
    fprintf(fout, "%s|", selection);
    fprintf(fout, "%ld|", elapsed);
    fprintf(fout, "%g|", score);
    // TODO: Check that | isn't in call string
    for (int i=0; i<argc; i++) fprintf(fout, "%s ", argv[i]);
    fprintf(fout, "\n");


    // TODO: add field for exercise specific data
    fclose(fout);
  }

  // Cleanup
  // TODO: free configs dictionary
  exercise_free(&exercise);

}

static void set_defaults(dict_T configs) {
  
  dict_set(configs, "exercise_dir", DEFAULT_EXERCISE_DIR);
  dict_set(configs, "exercise", DEFAULT_EXERCISE);

}

char *timestamp(char *buf, size_t len) {

  time_t now = time(NULL);
  if (!strftime(buf, len, "%F %T", localtime(&now))) return NULL;
  return buf;

}
