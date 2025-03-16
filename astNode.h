#ifndef _astNode_h_
#define _astNode_h_

#include "common.h"
#include <stdint.h>
#include <math.h>     // needed for float_t and double_t
#include <stdbool.h>



struct astNode
{
  uint32_t type;
  uint32_t flags;

  struct pos pos;

  union
  {
	struct exp
	{
	  struct astNode* left;
	  struct astNode* right;
	  char op[3];                 // allow for multi-token operators, eg. ++ or ->
	} exp;

	struct stmt
	{
	  uint32_t type;

	  struct returnStmt
	  {
		struct astNode* exp;
	  } returnStmt;
	} stmt;

	struct fnct
	{
		char* retType;
		char* name;
		struct vec* args;
		struct vec* stmts;      // vector of AST for each expression in function
	} fnct;

	struct prog
	{
		struct vec* fncts;
	} prog;
	
  };  // end of union

  union                            // is astNode is holding a literal value
  {
	  uint8_t       cVal;          // character literal
	  const char*   sVal;          // string literal
	  uint32_t      iVal;          // 32-bit integer literal
	  uint64_t      lVal;          // 64-bit integer literal
	  double_t      rVal;          // real literal
  };
};




struct astNode* astNode_create(struct astNode*);
void astNode_delete(struct astNode*);
void astNode_print(struct astNode*, uint32_t);
#endif
