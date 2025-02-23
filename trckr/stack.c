#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <trckr.h>

struct trckr_stack {
	int length;
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

int
trckr_stack_iterate(struct trckr_stack* stack, int (*callback)(void* item))
{
	assert(callback != NULL);
	assert(stack != NULL);

	if (stack->length == 0) {
		return 0;
	}

	struct trckr_stack_item* item = stack->first;
	while (item != NULL)
	{
		int result = callback(item->data);
		if (result != 0) {
			if (result == TRCKR_ITERATION_DONE) {
				return 0;
			}
			return result;
		}

		item = item->next;
	}

	return 0;
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
	return stack;
}