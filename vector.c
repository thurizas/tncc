#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


struct vector
{
  void* data;
  struct vector* flink;
  struct vector* blink;
};

void vec_init(struct vec** v)
{
  (*v)->head = NULL;
  (*v)->tail = NULL;
}

void vec_free(struct vec* v)
{
  if((v->tail != NULL) && (v->head != NULL))              // list is note empty
  {
	struct vector* t = v->tail;
	do
	{
	  v->tail = t->blink;
	  if(t->data != NULL) free(t->data);
	  free(t);
	} while((t = v->tail) != NULL);
  }

  free(v);
}

void vec_append(struct vec* v, void* data)
{
  if((v->head == NULL) && (v->tail == NULL))
  {
	struct vector* temp = calloc(1, sizeof(struct vector));
	temp->data = data;
	temp->flink = NULL;
	temp->blink = NULL;
	
	v->head = temp;
	v->tail = temp;
  }
  else
  {
	struct vector* t = v->head;
	while((t = t->flink) != NULL);

	struct vector* temp = calloc(1, sizeof(struct vector));
	temp->data = data;
	temp->flink = NULL;
	temp->blink = v->tail;
	v->tail->flink = temp;
	v->tail = temp;
  }
}


void vec_print(struct vec* v)
{
  if((v->head != NULL) && (v->tail != NULL))
  {
	struct vector* t = v->head;
	do
	{
	  if(t->data != NULL)
	  {
		fprintf(stderr, "%c ->", *((char*)(t->data)) );
		t = t->flink;
	  }
	} while(t != NULL);
	fprintf(stderr, "\n");
  }
  else
  {
	fprintf(stderr, "[+] list is empty\n");
  }
}
