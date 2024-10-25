#include "buffer.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const uint32_t growSize = 100;
static const uint32_t initSize = 50;
static const float_t  growPoint = 0.75;

struct buffer
{
  uint32_t maxSize;
  uint32_t curSize;
  uint32_t peekPtr;

  char*    data;
};



void buf_init(struct buffer** ppBuf)
{
  if(*ppBuf != NULL)
  {
	free(*ppBuf);
  }

  *ppBuf = calloc(1, sizeof(struct buffer));
  fprintf(stderr, "[+] allocating buffer structure at 0x%8p\n", *ppBuf);
  
  (*ppBuf)->maxSize = initSize;
  (*ppBuf)->curSize = 0;
  (*ppBuf)->data = calloc(initSize, sizeof(char));
}

void buf_free(struct buffer** pbuf)
{
  if((*pbuf)->data != NULL)
  {
	free((*pbuf)->data);
	fprintf(stderr, "[+] freeing internal memory at 0x%8p\n", (*pbuf)->data);
  }

  (*pbuf)->curSize = 0;
  (*pbuf)->peekPtr = 0;

  free(*pbuf);
  fprintf(stderr, "[+] freeing buffer structure at 0x%8p\n", *pbuf);
  *pbuf = NULL;
}

int buf_len(struct buffer* pbuf)
{
  return pbuf->curSize - 1;            // curSize points to where next character will go
}

void buf_setPeekPtr(struct buffer* pbuf, int32_t loc)
{
  pbuf->peekPtr = loc;
}

void buf_append(struct buffer* pbuf, char ch)
{
  uint32_t growSize = floor(growPoint*(pbuf->maxSize));
  if(pbuf->curSize <= growSize)
  {
	pbuf->data[pbuf->curSize] = ch;
	pbuf->curSize++;
  }
  else                // grow the buffer by growSize
  {
	int32_t newSize = pbuf->maxSize + growSize;
	char* temp = calloc(1, newSize);
	fprintf(stderr, "[+] allocating %d bytes of internal memory ot 0x%8p\n", newSize, temp);
	memcpy(temp, pbuf->data, pbuf->curSize);
	free(pbuf->data);
	fprintf(stderr, "[+] freeing internal memory at 0x%8p\n", pbuf->data);
	pbuf->maxSize = newSize;
	pbuf->data = temp;
  }
}


char buf_peek(struct buffer* pbuf)
{
  char ch;
  
  if(pbuf->peekPtr < pbuf->curSize)
	ch = pbuf->data[pbuf->peekPtr];

  return ch;
}

// pop and element from the buffer, actually just decrease its size by one
char buf_pop(struct buffer* pbuf)
{
  char ch = 0x00;
  
  if(pbuf->curSize >= 1)
  {
	ch = pbuf->data[pbuf->curSize-1];
	pbuf->curSize--;
  }

  return ch;
}

char buf_at(struct buffer* pbuf, uint32_t loc)
{
  char ch=0x00;

  if((loc >= 0) && (loc < pbuf->curSize))
  {
	ch = pbuf->data[loc];
  }

  return ch;
}

void buf_print(struct buffer* pbuf)
{
  if(pbuf != NULL)
  {
	if(pbuf->data != NULL)
	{
	  for(uint32_t ndx = 0; ndx < pbuf->curSize; ndx++)
	  {
		fprintf(stdout, "%c", pbuf->data[ndx]);
	  }
	  fprintf(stdout, "\n");		  
	}
	else
	{
	  fprintf(stdout, "[+] string is empty\n");
	}
  }
  else
  {
	fprintf(stdout, "[+] string does not exis\n");
  }
}
