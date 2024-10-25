#ifndef _vector_h_
#define _vector_h_

struct vector;

struct vec
{
  struct vector* head;
  struct vector* tail;
};

void vec_init(struct vec**);
void vec_free(struct vec*);

void vec_append(struct vec*, void*);

void vec_print(struct vec*);


#endif
