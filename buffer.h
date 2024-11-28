// a variable length string.
// initial size is set to 200, and growth is by 100
#ifndef _buffer_h_
#define _buffer_h_

#include <stdint.h>
#include <stdbool.h>

//struct buffer;
struct buffer
{
  uint32_t maxSize;
  uint32_t curSize;
  uint32_t peekPtr;

  char*    data;
};
void buf_init(struct buffer**);
void buf_free(struct buffer**);

int  buf_len(struct buffer*);

void buf_setPeekPtr(struct buffer*, int32_t);

void buf_append(struct buffer*, char);
void buf_insert(struct buffer*, const char*);
char buf_peek(struct buffer*);
char buf_pop(struct buffer*);

char buf_at(struct buffer*, int);

char* buf_data(struct buffer*);

void buf_print(struct buffer*);





#endif
