//
// -----------------------------------------------------------------------------
// deque.c
// -----------------------------------------------------------------------------
//

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <error.h>
#include <mem.h>
#include "deque.h"

#define D Deque_T

struct node {
  struct node *llink, *rlink;
  void *value;
};

struct D {
  struct node *head;
  struct node *tail;
  int length;
};

D Deque_new() {
  D deque;

  NEW0(deque);
  deque->head = deque->tail = NULL;
  return deque;
}

// TODO: fix a bug in the va_args that continues to read past the last elem
D Deque_deque(void *x, ...) {
  va_list ap;
  D deque = Deque_new();

  va_start(ap, x);
  for ( ; x; x = va_arg(ap, void *))
    Deque_addhi(deque, x);
  va_end(ap);
  return deque;
}

void Deque_free(D *deque) {
  assert(deque && *deque);

  struct node *p, *q;

  if ((p = (*deque)->head) != NULL) {
    int n = (*deque)->length;
    for ( ; n-- > 0; p = q) {
      q = p->rlink;
      FREE(p);
    }
  }
  FREE(*deque);
}

int Deque_length(D deque) {
  assert(deque);
  return deque->length;
}

void *Deque_get(D deque, int i) {

  assert(deque);
  assert(i >= 0 && i < deque->length);

  struct node *q = deque->head;

  for ( int n=0; n < i; n++) q = q->rlink;

  return q->value;
}

void *Deque_put(D deque, int i, void *x) {
  assert(deque);
  assert(i >= 0 && i < deque->length);

  struct node *q = deque->head;

  for ( int n=0; n<i; n++ ) q = q->rlink;
  void *prev = q->value;
  q->value = x;

  return prev;
}

void *Deque_addlo(D deque, void *x) {
  assert(deque);

  struct node *new, *head = deque->head;
  NEW(new);

  if (head != NULL) {
    new->rlink = head;
    deque->head = head->llink = new;
  } else
    deque->head = deque->tail = new;

  deque->length++;
  return new->value = x;
}

void *Deque_addhi(D deque, void *x) {

  assert(deque);

  struct node *new, *tail = deque->tail;
  NEW(new);

  if (tail != NULL) {
    new->llink = tail;
    deque->tail = tail->rlink = new;
  } else
    deque->head = deque->tail = new;

  deque->length++;
  return new->value = x;
}
    
void *Deque_remlo(D deque) {

  assert(deque && deque->length > 0);

  struct node *q = deque->head;
  void *x = q->value;

  deque->head = q->rlink;

  if (--deque->length == 0)
    deque->head = deque->tail = NULL;
  else
    q->rlink->llink = NULL;

  FREE(q);

  return x;
}

void *Deque_remhi(D deque) {
  assert(deque && deque->length > 0);

  struct node *q = deque->tail;
  void *x = q->value;

  deque->tail = q->llink;

  if (--deque->length == 0)
    deque->head = deque->tail = NULL;
  else
    q->llink->rlink = NULL;

  FREE(q);

  return x;
}

void Deque_map(D deque, void apply(void **x, void *cl), void *cl) {
  assert(apply);
  struct node *q = deque->head;

  for ( ; q ; q = q->rlink )
    apply(&q->value, cl);
}
