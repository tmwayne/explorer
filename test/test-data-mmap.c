//
// -----------------------------------------------------------------------------
// test-data-mmap.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdio.h>
#include "error.h"
#include "minunit.h"
#include "frame.h"

int tests_run = 0;

// Data_T Data_mmap_init(char *path, char delim, int headers);
static char *test_Data_init_valid() {
  Data_T data = Data_mmap_init("path.csv", '|');
  mu_assert("Data_mmap_init returned NULL", data);
}

// void   Data_mmap_free(Data_T *data);
static char *test_Data_free_valid() {
  Data_T data = Data_mmap_init("path.csv", '|');
  Data_mmap_free(&data);
  mu_assert("Data_mmap_free didn't set data to NULL", !data);
}

static char *test_Data_free_throw_NULL_arg() {
  unsigned char pass = 0;
  TRY Data_mmap_free(NULL);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert("Data_mmap_free didn't throw error when passed NULL", pass);
}

static char *test_Data_free_throw_NULL_data() {
  unsigned char pass = 0;
  Data_T data = NULL;
  TRY Data_mmap_free(&data);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert("Data_mmap_free didn't throw error when passed NULL data", pass);
}

static char *test_Data_free_throw_NULL_data_args() {
  unsigned char pass = 0;
  Data_T data = Data_mmap_init("path.csv", '|');
  data->args = NULL;
  TRY Data_mmap_free(&data);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert(
    "Data_mmap_free didn't throw error when passed NULL data->args",
    pass);
}

static char* run_all_tests() {

  char *(*all_tests[])() = {
    test_Data_init_valid,
    test_Data_free_valid,
    test_Data_free_throw_NULL_arg,
    test_Data_free_throw_NULL_data,
    test_Data_free_throw_NULL_data_args,
    NULL
  };

  // Returns message of first failing test
  mu_run_all(all_tests);
    
  return 0;
}

int main(int argc, char** argv) {
  char* result = run_all_tests();
  if (result != 0) printf("%s\n", result);
  else printf("ALL TESTS PASSED\n");
  printf("Tests run: %d\n", tests_run);
  return result != 0;
}
