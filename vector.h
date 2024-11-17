#ifndef _vector_h_
#define _vector_h_

#include <stdint.h>

struct node;

struct vec
{
  int32_t      curNdx;            // which element is the current elements
  uint32_t     cntItems;          // number of items managed by this vector
  struct node* curItem;           // pointer to the current item
  struct node* head;
  struct node* tail;
};

void vec_init(struct vec**);
void vec_free(struct vec*);

void vec_setCurrentNdx(struct vec*, int32_t);

void* vec_peekCurrent(struct vec* v);     // peeks at current value, does not modify current index
void* vec_getCurrent(struct vec* v);
void vec_pop(struct vec* v);             // return current value, and move current index up one



void vec_push(struct vec*, void*);

void vec_print(struct vec*, void (*ptr)(void*));

void tokens_clear(struct vec*);

#endif
