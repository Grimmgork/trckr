#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <trckr.h>

struct trckr_stack {
	int length;
	struct trckr_stack_item *iterator;
	struct trckr_stack_item *first;
	struct trckr_stack_item *last;
};

struct trckr_stack_item {
	struct trckr_stack *stack;
	struct trckr_stack_item *next;
	char data[1];
};

void*
trckr_stack_push(struct trckr_stack* stack, size_t size)
{
	assert(size > 0);

	struct trckr_stack_item* item = malloc(sizeof(struct trckr_stack_item) + size - 1);
	if (item == NULL) {
		return NULL;
	}

	item->stack = stack;
	item->next = NULL;
	if (stack->length == 0)
	{
		stack->length = 1;
		stack->first = item;
		stack->last = item;
	}
	else
	{
		stack->length++;
		stack->last->next = item;
		stack->last = item;
	}

	return item->data;
}

void*
trckr_stack_iterate_next(struct trckr_stack* stack)
{
	assert(stack->iterator != NULL);

	stack->iterator = stack->iterator->next;
	return stack->iterator;
}

void*
trckr_stack_iterate_reset(struct trckr_stack* stack)
{
	stack->iterator = stack->first;
	return stack->iterator;
}

int
trckr_stack_free(struct trckr_stack* stack)
{
	if (stack->length == 0) {
		return 0;
	}

	struct trckr_stack_item* item = stack->first;
	while (item != NULL)
	{
		struct trckr_stack_item* to_free = item;
		item = item->next;
		free(to_free);
	}

	return 0;
}

struct trckr_stack*
trckr_stack_init()
{
	struct trckr_stack* stack = malloc(sizeof(struct trckr_stack));
	stack->length = 0;
	stack->first = NULL;
	stack->last = NULL;
	stack->iterator = NULL;
	return stack;
}