//
// -----------------------------------------------------------------------------
// test_deque.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdio.h>
#include <error.h>
#include "minunit.h"
#include "deque.h"

int tests_run = 0;

/*
D     Deque_new    (void);
D     Deque_deque  (void *, ...);
void  Deque_free   (D *);
int   Deque_length (D);
void *Deque_get    (D, int);
void *Deque_put    (D, int, void *); 
void *Deque_addlo  (D, void *); 
extern void *Deque_addhi  (D, void *);
void *Deque_remlo  (D);
void *Deque_remhi  (D); 
void  Deque_map    (D deque, void apply(void **x, void *cl), void *cl);
*/

char *test_Deque_new_valid() {
  Deque_T deque = Deque_new();
  mu_assert("error: Deque_new returned NULL", deque);
}

char *test_Deque_new_length_0() {
  Deque_T deque = Deque_new();
  mu_assert("error: length of Deque_new not 0", Deque_length(deque) == 0);
}

char *test_Deque_free_valid() {
  Deque_T deque = Deque_new();
  Deque_free(&deque);
  mu_assert("error: deque wasn't NULL after Deque_free", deque == NULL);
}

char *test_Deque_free_throws_NULL_arg() {
  int pass = 0;
  TRY Deque_free(NULL);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert("error: Deque_free didn't throw when given NULL argument", pass);
}

char *test_Deque_free_throws_NULL_deque() {
  int pass = 0;
  Deque_T deque = NULL;
  TRY Deque_free(&deque);
  EXCEPT (Assert_Failed) pass = 1;
  END_TRY;
  mu_assert("error: Deque_free didn't throw when given NULL deque", pass);
}

char* run_all_tests() {

  char *(*all_tests[])() = {
    test_Deque_new_valid,
    test_Deque_new_length_0,
    test_Deque_free_valid,
    test_Deque_free_throws_NULL_arg,
    test_Deque_free_throws_NULL_deque,
    NULL
  };

  // Returns the message of the first test that fails
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
