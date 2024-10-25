#ifndef _common_h_
#define _common_h_

static const uint8_t FLAGS_DEBUG   = 0x01;
static const uint8_t FLAGS_LEX     = 0x02;
static const uint8_t FLAGS_PAR     = 0x04;
static const uint8_t FLAGS_CODEGEN = 0x08;
static const uint8_t FLAGS_ALL     = 0x0E;

static const uint8_t LINE_BEGIN    = 0x01;
static const uint8_t LINE_COMMENT  = 0x02;
static const uint8_t BLCK_COMMENT  = 0x04;

static const uint8_t MAJOR = 0;
static const uint8_t MINOR = 1;
static const uint8_t PATCH = 0;



enum
{
	ERR_LEX_FAILED = 1,
	ERR_PARSE_FAILED
};

#endif
