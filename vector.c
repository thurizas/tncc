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
	  //if(t->data != NULL) free(t->data);
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

// get number of items managed by this vector
uint32_t vec_len(struct vec* v)
{
	return v->cntItems;
}

// peeks at next value, does not change current index
void* vec_peek(struct vec* v)
{
	return v->curItem->flink;
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
	//else
	//{
	//	fprintf(stderr, "[-] no current item selected, call setCurrentNdx first\n");
	//}
	return NULL;
}

// pushes to the end of the link list
void vec_push(struct vec* v, int size, void* data)
{
  struct node* temp = calloc(1, sizeof(struct node));
  if (NULL == temp)
  {
	  fprintf(stderr, "[-] failed to allocate new node for vector\n");
  }
  else
  {
	if((v->head == NULL) && (v->tail == NULL))
	{
		temp->dataSize = size;
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

		temp->dataSize = size;
		temp->data = data;
		temp->flink = NULL;
		temp->blink = v->tail;
		v->tail->flink = temp;
		v->tail = temp;
	}

	v->cntItems++;
  }
}

// pushes to the front of the vector
void vec_front(struct vec* v, int size, void* data)
{		
	struct node* temp = calloc(1, sizeof(struct node));
	if (NULL == temp)
	{
		fprintf(stderr, "[-] failed to allocate new node for vector\n");
	}
	else
	{
		if ((v->head == NULL) && (v->tail == NULL))                    // list is empty
		{
			temp->dataSize = size;
			temp->data = data;
			temp->flink = NULL;
			temp->blink = NULL;

			v->head = temp;
			v->tail = temp;
		}
		else                                                          // list has data
		{
			temp->dataSize = size;
			temp->data = data;
			temp->blink = NULL;
			temp->flink = v->head;

			v->head->blink = temp;
			v->head = temp;
		}

		v->cntItems++;
	}
}

void vec_print(struct vec* v, void (*ptr)(void*), bool blink)
{
  if((v->head != NULL) && (v->tail != NULL))
  {
	struct node* t = v->head;
	do
	{
	  if(t->data != NULL)
	  {
		ptr(t->data);
		if(blink) fprintf(stdout, " ->");
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

void vec_clear(struct vec* v)
{
	if (v->cntItems > 0)
	{
		vec_setCurrentNdx(v, 0);

		struct node* node = vec_getCurrent(v);

		while (NULL != node)
		{
			free(node->data);

			struct node* nextNode = vec_getCurrent(v);
			free(node);
			node=nextNode;
		}

		v->cntItems = 0;
		v->curItem = NULL;
		v->curNdx = -1;

		v->head = NULL;
		v->tail = NULL;
	}
}


void vec_copy(struct vec* dst, struct vec* src)
{
	if (src->cntItems > 0)                           // we have stuff to copy
	{
		if (dst->cntItems > 0)                       // if dst has stuff in it, clean
		{
			vec_clear(dst);
		}

		struct node* temp = src->head;
		while (NULL != temp)
		{
			int len = temp->dataSize;
			void* data = calloc(len + 1, sizeof(char));
			if (NULL != data)
			{
				memcpy(data, temp->data, len);
				vec_push(dst, len, (void*)data);
			}
			else
			{
				fprintf(stderr, "[-] unable to allocate memory in vec_copy\n");
			}

			temp = temp->flink;
		}

	}
}

// TODO : does this belong here...should be moved to token.c (?)
void tokens_clear(struct vec* tokens)
{
  struct token*    token;

  if(tokens->cntItems > 0)
  {
	vec_setCurrentNdx(tokens, 0);

	token = vec_getCurrent(tokens);

	while (NULL != token)
	{
	  if ((token->type == TOKEN_TYPE_ID) || (token->type == TOKEN_TYPE_KEYWORD) ||
		  (token->type == TOKEN_TYPE_TYPE) || (token->type == TOKEN_TYPE_STRING))
	  {
		  free(token->sVal);
	  }

	  struct token* nextToken = vec_getCurrent(tokens);
	  free(token);
	  token = nextToken;
	}
  }
}
