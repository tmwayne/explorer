// 
// -----------------------------------------------------------------------------
// deque.h
// -----------------------------------------------------------------------------
//
// Double-ended Queue ADT.
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
