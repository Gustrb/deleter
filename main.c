#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_CHILDREN_PER_NODE 32
#define MAX_NUMBER_OF_NODES 32 * 1024

typedef struct {
	const char *id;
	const char *name;
} parent_entity_t;

typedef struct {
	const char *name;
} child_entity;

typedef struct entity_node_t {
	size_t children_idxs[MAX_CHILDREN_PER_NODE];
	union {
		parent_entity_t e;
		child_entity_t c;
	} as;
	uint8_t kind;
} entity_node_t;

static entity_node_t pool[MAX_NUMBER_OF_NODES] = {0};
static size_t pool_len = 0;

int main(void)
{
	return 0;
}
