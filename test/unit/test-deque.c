//
// -----------------------------------------------------------------------------
// test_deque.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdio.h>
#include "error.h"
#include "minunit.h"
#include "deque.h"

int tests_run = 0;

// Deque_T Deque_new();
static char *test_Deque_new_valid() {
  Deque_T deque = Deque_new();
  mu_assert("Deque_new returned NULL", deque);
}

static char *test_Deque_new_length_0() {
  Deque_T deque = Deque_new();
  mu_assert("Length of Deque_new not 0", Deque_length(deque) == 0);
}

// void Deque_free(Deque_T *deque);
static char *test_Deque_free_valid() {
  Deque_T deque = Deque_new();
  Deque_free(&deque);
  mu_assert("Deque wasn't NULL after Deque_free", deque == NULL);
}

static char *test_Deque_free_throws_NULL_arg() {
  unsigned char pass = 0;
  TRY Deque_free(NULL);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert("Deque_free didn't throw when given NULL argument", pass);
}

static char *test_Deque_free_throws_NULL_deque() {
  unsigned char pass = 0;
  Deque_T deque = NULL;
  TRY Deque_free(&deque);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert("Deque_free didn't throw when given NULL deque", pass);
}

// TODO: write tests for the following functions
// Deque_T     Deque_deque  (void *, ...);
// int   Deque_length (Deque_T);
// void *Deque_get    (Deque_T, int);
// void *Deque_put    (Deque_T, int, void *); 
// void *Deque_addlo  (Deque_T, void *); 
// extern void *Deque_addhi  (Deque_T deque, void *);
// void *Deque_remlo  (Deque_T);
// void *Deque_remhi  (Deque_T); 
// void  Deque_map    (Deque_T, void apply(void **x, void *cl), void *cl);

static char* run_all_tests() {

  char *(*all_tests[])() = {
    test_Deque_new_valid,
    test_Deque_new_length_0,
    test_Deque_free_valid,
    test_Deque_free_throws_NULL_arg,
    test_Deque_free_throws_NULL_deque,
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
