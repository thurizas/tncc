#include "buffer.h"
#include "common.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const uint32_t initSize = 50;
static const float_t  growPoint = 0.75;


void buf_init(struct buffer** ppBuf)
{
  if(*ppBuf != NULL)
  {
	free(*ppBuf);
  }

  *ppBuf = calloc(1, sizeof(struct buffer));
  //fprintf(stderr, "[+] allocating buffer structure in %s at 0x%8p\n", __func__, (void*)*ppBuf);
  
  (*ppBuf)->maxSize = initSize;
  (*ppBuf)->curSize = 0;
  (*ppBuf)->data = calloc(initSize, sizeof(char));
  //fprintf(stderr, "allocating internal memory in %s at 0x%8p\n", __func__, (*ppBuf)->data);
}

void buf_free(struct buffer** pbuf)
{
  if((*pbuf)->data != NULL)
  {
	free((*pbuf)->data);
	//fprintf(stderr, "[+] freeing internal memory in %s at 0x%8p\n", __func__,  (*pbuf)->data);
  }

  (*pbuf)->curSize = 0;
  (*pbuf)->peekPtr = 0;

  free(*pbuf);
  //fprintf(stderr, "[+] freeing buffer structure in %s at 0x%8p\n", __func__, (void*)*pbuf);
  *pbuf = NULL;
}

int buf_len(struct buffer* pbuf)
{
  return pbuf->curSize;                 
}

void buf_setPeekPtr(struct buffer* pbuf, int32_t loc)
{
  pbuf->peekPtr = loc;
}

void buf_append(struct buffer* pbuf, char ch)
{
  uint32_t growSize = (uint32_t)floor(growPoint*(pbuf->maxSize));
  if(pbuf->curSize > growSize)              // grow the buffer by growSize
  {
	  int32_t newSize = pbuf->maxSize + growSize;
	  char* temp = NULL;
	  if (NULL != (temp = calloc(1, newSize)))
	  {
		//fprintf(stderr, "[+] allocating %d bytes of internal memory in %s at 0x%8p\n", newSize, __func__, temp);
		memcpy(temp, pbuf->data, pbuf->curSize);
		free(pbuf->data);
		//fprintf(stderr, "[+] freeing internal memory in %s at 0x%8p\n", __func__, pbuf->data);
		pbuf->maxSize = newSize;
		pbuf->data = temp;
	  }
	  else
	  {
		  fprintf(stderr, "[-] failed to reallocate memory for input string");
		  exit(-ERR_MEMORY);
	  }

  }
  
  pbuf->data[pbuf->curSize] = ch;
  pbuf->curSize++; 
}


char buf_peek(struct buffer* pbuf)
{
  char ch=0x00;
  
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

char buf_at(struct buffer* pbuf, int loc)
{
  char ch=0x00;

  if((loc >= 0) && (loc < (int)pbuf->curSize))
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

char* buf_data(struct buffer* buf)
{
	return buf->data;
}
