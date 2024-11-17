#include "vector.h"
#include "common.h"
#include "node.h"


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>



void vec_init(struct vec** v)
{

  if (*v != NULL)
  {
	vec_free(*v);
	free(*v);
  }

  if (NULL != (*v = malloc(sizeof(struct vec))))
  {
	(*v)->head = NULL;
	(*v)->tail = NULL;
	(*v)->curItem = NULL;
	(*v)->curNdx = -1;
	(*v)->cntItems = 0;
  }
}

void vec_free(struct vec* v)
{
  if((v->tail != NULL) && (v->head != NULL))              // list is note empty
  {
	struct node* t = v->tail;
	do
	{
	  v->tail = t->blink;
	  if(t->data != NULL) free(t->data);
	  free(t);
	} while((t = v->tail) != NULL);
  }

  free(v);
}

void vec_setCurrentNdx(struct vec* v, int32_t ndx)
{
	(*v).curNdx = ndx;
	v->curItem = v->head;
}

// peeks at next value, does not change current index
void* vec_peek(struct vec* v)
{
	return NULL;
}

// return current value, and move current index up one
void vec_pop(struct vec* v)
{
	if (v->curNdx < v->cntItems)
	{
		v->curNdx += 1;
		v->curItem = v->curItem->flink;
	}
		
}

// combines the vec_peekCurrent() and vec_pop() operation into a single function
void* vec_getCurrent(struct vec* v)
{
	void* node = vec_peekCurrent(v);
	vec_pop(v);

	return node;
}

// peeks at current token, does not modify current index
//struct node* vec_peekCurrent(struct vec* v)
void* vec_peekCurrent(struct vec* v)
{
	//struct node* temp = NULL;
	struct node* temp = NULL;

	if (v->curItem != NULL)
	{
		temp = v->curItem;
		return temp->data;
	}
	else
	{
		fprintf(stderr, "[-] no current item selected, call setCurrentNdx first\n");
	}
	return NULL;
}

void vec_push(struct vec* v, void* data)
{
  if((v->head == NULL) && (v->tail == NULL))
  {
	struct node* temp = calloc(1, sizeof(struct node));
	temp->data = data;
	temp->flink = NULL;
	temp->blink = NULL;
	
	v->head = temp;
	v->tail = temp;
  }
  else
  {
	struct node* t = v->head;
	while((t = t->flink) != NULL);

	struct node* temp = calloc(1, sizeof(struct node));
	temp->data = data;
	temp->flink = NULL;
	temp->blink = v->tail;
	v->tail->flink = temp;
	v->tail = temp;
  }

  v->cntItems++;
}

void vec_print(struct vec* v, void (*ptr)(void*))
{
  if((v->head != NULL) && (v->tail != NULL))
  {
	struct node* t = v->head;
	do
	{
	  if(t->data != NULL)
	  {
		ptr(t->data);
		fprintf(stdout, " ->");
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

//  This function clears the token buffer, deleting each token in the buffer.  
//  The actual buffer is destroyed in lexer_deinit().
void tokens_clear(struct vec* tokens)
{
  struct node*    node;
  if(tokens->cntItems > 0)
  {
	node = tokens->head;

	while(NULL != node)
	{
	  struct token* t = (struct token*)node->data;

	  // do we have a sVal to get rid of
	  if((t->type == TOKEN_TYPE_ID) || (t->type == TOKEN_TYPE_KEYWORD) ||
		 (t->type == TOKEN_TYPE_TYPE) || (t->type == TOKEN_TYPE_STRING))
	  {
		free(t->sVal);
	  }

	  node = node->flink;
	}
  }
}
