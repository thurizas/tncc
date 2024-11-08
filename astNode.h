#ifndef _astNode_h_
#define _astNode_h_

#include "common.h"
#include <stdint.h>
#include <stdbool.h>



struct astNode
{
  uint32_t type;
  uint32_t flags;

  struct pos;

  union
  {
	struct exp
	{
	  struct node* left;
	  struct node* right;
	  const char* op;
	} exp;

	struct statement
	{
	  struct returnStmt
	  {
		struct node* exp;
	  } returnStmt;
	} stmt;
	
  };  // end of union
};

#endif
