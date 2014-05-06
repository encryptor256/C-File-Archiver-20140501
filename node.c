#include "node.h"

Node_t * Node_new(void)
{
	return calloc(1,sizeof(Node_t));
};
