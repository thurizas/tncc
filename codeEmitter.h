#ifndef _codeEmitter_h_
#define _codeEmitter_h_

#include "vector.h"
#include "buffer.h"

#include <stdint.h>
#include <stdbool.h>

bool ce_init(struct vec*, const char*, uint8_t);
void ce_deinit();

bool ce_emit();

#endif

