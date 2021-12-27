// 
// -----------------------------------------------------------------------------
// deque.h
// -----------------------------------------------------------------------------
//
// Double-ended Queue ADT.
//
// Tyler Wayne © 2021
//

#ifndef DEQUE_INCLUDED
#define DEQUE_INCLUDED

#define D Deque_T
typedef struct D *D;

extern D     Deque_new    (void);
extern D     Deque_deque  (void *, ...);
extern void  Deque_free   (D *);
extern int   Deque_length (D);
extern void *Deque_get    (D, int);
extern void *Deque_put    (D, int, void *); 
extern void *Deque_addlo  (D, void *); 
extern void *Deque_addhi  (D, void *);
extern void *Deque_remlo  (D);
extern void *Deque_remhi  (D); 
extern void  Deque_map    (D deque, void apply(void **x, void *cl), void *cl);

#undef D
#endif // DEQUE_INCLUDED
