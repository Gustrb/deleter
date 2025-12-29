#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
} child_entity_t;

#define KIND_PARENT 0x1
#define KIND_CHILD  0x2

typedef struct entity_node_t {
	size_t children_idxs[MAX_CHILDREN_PER_NODE];
	union {
		parent_entity_t parent;
		child_entity_t child;
	} as;
	size_t num_children;
	uint8_t kind;
} entity_node_t;

entity_node_t pool[MAX_NUMBER_OF_NODES] = {0};
size_t pool_len = 0;

entity_node_t *deleter_new_cascading_subtree(const char *root_name, const char *root_id);
entity_node_t *deleter_add_parent_subtree(entity_node_t *e, const char *name, const char *id);
entity_node_t *deleter_add_child_subtree(entity_node_t *e, const char *name);
entity_node_t *deleter_find_subtree_by_id(entity_node_t *e, const char *name, const char *id);

void deleter_debug_tree(entity_node_t *root);

int main(void)
{
	entity_node_t *root = deleter_new_cascading_subtree("organizations", "1");
	if (root == NULL)
	{
		fprintf(stderr, "[ERROR]: Failed to allocate a new entry\n");
		return 1;
	}
	entity_node_t *open_job_roles = deleter_add_parent_subtree(root, "open_job_roles", "100");

	entity_node_t *role = deleter_find_subtree_by_id(open_job_roles,"open_job_roles", "100");
	deleter_add_parent_subtree(root, "users", "100");
	deleter_add_child_subtree(open_job_roles, "resumes");
	deleter_debug_tree(role);

	return 0;
}

entity_node_t *deleter_new_cascading_subtree(const char *root_name, const char *root_id)
{
	assert(root_name != NULL);
	assert(root_id != NULL);

	// Pool full I guess...
	if (pool_len >= MAX_NUMBER_OF_NODES) return NULL;

	entity_node_t *n = &pool[pool_len++];
	n->num_children = 0;
	n->kind = KIND_PARENT;
	n->as.parent.name = root_name;
	n->as.parent.id = root_id;

	return n;
}

void __deleter_debug_tree_inner(entity_node_t *root, uint8_t tabbing)
{
	char tabs[256] = {0};
	memset(tabs, '\t', tabbing);
	
	if (root->kind == KIND_PARENT)
	{
		fprintf(stdout, "%sparent(%s, %s)\n", tabs, root->as.parent.name, root->as.parent.id);
	}
	else
	{
		fprintf(stdout, "%schild(%s)\n", tabs, root->as.child.name);
	}

	for (size_t i = 0; i < root->num_children; i++)
	{
		size_t child_idx = root->children_idxs[i];
		__deleter_debug_tree_inner(&pool[child_idx], tabbing+1);
	}
}

void deleter_debug_tree(entity_node_t *root)
{
	__deleter_debug_tree_inner(root, 0);
}

entity_node_t *deleter_add_parent_subtree(entity_node_t *parent, const char *name, const char *id)
{
	assert(parent != NULL);
	assert(name != NULL);
	assert(id != NULL);

	size_t new_node_idx = pool_len;
	entity_node_t *new_node = deleter_new_cascading_subtree(name, id);
	if (new_node == NULL) return NULL;

	if (parent->num_children >= MAX_CHILDREN_PER_NODE) return NULL;
	parent->children_idxs[parent->num_children++] = new_node_idx;

	new_node->num_children = 0;
	new_node->kind = KIND_PARENT;
	new_node->as.parent.name = name;
	new_node->as.parent.id = id;

	return new_node;
}

entity_node_t *deleter_add_child_subtree(entity_node_t *parent, const char *name)
{
	assert(parent != NULL);
	assert(name != NULL);

	size_t new_node_idx = pool_len;
	entity_node_t *new_node = deleter_new_cascading_subtree(name, "");
	if (new_node == NULL) return NULL;

	if (parent->num_children >= MAX_CHILDREN_PER_NODE) return NULL;
	parent->children_idxs[parent->num_children++] = new_node_idx;

	new_node->num_children = 0;
	new_node->kind = KIND_CHILD;
	new_node->as.child.name = name;

	return new_node;
}

entity_node_t *deleter_find_subtree_by_id(entity_node_t *e, const char *name, const char *id)
{
	assert(e != NULL);
	assert(id != NULL);

	if (e->kind != KIND_PARENT) return NULL;

	size_t idlen = strlen(id);
	assert(idlen > 0);

	size_t namelen = strlen(name);
	assert(namelen > 0);

	uint8_t id_matches = strncmp(e->as.parent.id, id, idlen) == 0;
	uint8_t name_matches = strncmp(e->as.parent.name, name, namelen) == 0;
	if (id_matches && name_matches) return e;

	entity_node_t *node = NULL;
	for (size_t i = 0; i < e->num_children; ++i)
	{
		size_t child_idx = e->children_idxs[i];
		node = deleter_find_subtree_by_id(&pool[child_idx], name, id);		
	}

	return node;
}

