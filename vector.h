#ifndef _vector_h_
#define _vector_h_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct node;

struct vec
{
  int32_t      curNdx;            // which element is the current elements
  int32_t      cntItems;          // number of items managed by this vector
  struct node* curItem;           // pointer to the current item
  struct node* head;
  struct node* tail;
};

void vec_init(struct vec**);
void vec_free(struct vec*);

void vec_clear(struct vec*);

void vec_setCurrentNdx(struct vec*, int32_t);
uint32_t vec_len(struct vec* v);

// insertion/deletion function....
void vec_enqueue(struct vec*, size_t, void*);     // adds to tail of vector
void vec_push(struct vec*, size_t, void*);        // adds to head of vector
void vec_pop(struct vec* v); 

// item access functions
void* vec_getCurrent(struct vec* v);             // peeks at current value, does not modify current index
void* vec_peekNext(struct vec* v);                // peeks at next items
void* vec_getNext(struct vec* v);                 // advance to next element, return that value
               
void vec_copy(struct vec* dst, struct vec* src);  //
void vec_print(struct vec*, void (*ptr)(void*), bool);



#endif
