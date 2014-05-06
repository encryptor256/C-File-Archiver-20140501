#ifndef node_h
#define node_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

typedef struct tagNode_t
{
	
	u32 type;
	
	struct tagNode_t * parent, * first, * last, * prev, * next;
	
}Node_t;


#endif