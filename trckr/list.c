#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <trckr.h>

struct trckr_list {
	int length;
	struct trckr_list_item *iterator;
	struct trckr_list_item *first;
	struct trckr_list_item *last;
};

struct trckr_list_item {
	struct trckr_list *list;
	struct trckr_list_item *next;
	char data[1];
};

void*
trckr_list_push(struct trckr_list* list, size_t size)
{
	assert(size > 0);

	struct trckr_list_item* item = malloc(sizeof(struct trckr_list_item) + size - 1);
	if (item == NULL) {
		return NULL;
	}

	item->list = list;
	item->next = NULL;
	if (list->length == 0)
	{
		list->length = 1;
		list->first = item;
		list->last = item;
	}
	else
	{
		list->length++;
		list->last->next = item;
		list->last = item;
	}

	return item->data;
}

void*
trckr_list_iterate_next(struct trckr_list* list)
{
	assert(list->iterator != NULL);

	list->iterator = list->iterator->next;
	return list->iterator;
}

void*
trckr_list_iterate_reset(struct trckr_list* list)
{
	list->iterator = list->first;
	return list->iterator;
}

int
trckr_list_free(struct trckr_list* list)
{
	if (list->length == 0) {
		return 0;
	}

	struct trckr_list_item* item = list->first;
	while (item != NULL)
	{
		struct trckr_list_item* to_free = item;
		item = item->next;
		free(to_free);
	}

	return 0;
}

struct trckr_list*
trckr_list_init()
{
	struct trckr_list* list = malloc(sizeof(struct trckr_list));
	list->length = 0;
	list->first = NULL;
	list->last = NULL;
	list->iterator = NULL;
	return list;
}