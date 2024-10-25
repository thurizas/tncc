// a variable length string.
// initial size is set to 200, and growth is by 100
#ifndef _buffer_h_
#define _buffer_h_

#include <stdint.h>
#include <stdbool.h>

struct buffer;

void buf_init(struct buffer**);
void buf_free(struct buffer**);

int  buf_len(struct buffer*);

void buf_setPeekPtr(struct buffer*, int32_t);

void buf_append(struct buffer*, char);
char buf_peek(struct buffer*);
char buf_pop(struct buffer*);

char buf_at(struct buffer*, uint32_t);

void buf_print(struct buffer*);





#endif
