#include <stdio.h>   
#include <stdlib.h>
#include <assert.h>

#include <arena.h>

struct arena_chunk {
	struct arena_chunk* next;
	char data[ARENA_CHUNK_SIZE];
};

struct arena_scope {
	int data_offset;
	struct arena_chunk* head;
};

struct arena {
	int scope_index;
	struct arena_scope scopes[MAX_SCOPE_DEPTH];
	struct arena_chunk* last; // last allocated chunk
	struct arena_chunk first;
};

int arena_alloc_chunk(struct arena *arena);

struct arena*
arena_init()
{
	struct arena* arena = malloc(sizeof(struct arena));
	arena->scope_index = 0;
	arena->last = &arena->first;
	arena->scopes[0].data_offset = 0;
	arena->scopes[0].head = &arena->first;
	arena->first.next = NULL;
}

void
arena_push_scope(struct arena *arena)
{
	if (arena->scope_index >= MAX_SCOPE_DEPTH) {
		return;
	}

	struct arena_scope* scope = &arena->scopes[arena->scope_index];
	int data_offset = scope->data_offset;
	struct arena_chunk* head = scope->head;

	arena->scope_index++;
	scope = &arena->scopes[arena->scope_index];
	
	scope->data_offset = data_offset;
	scope->head = head;
}

void*
arena_push(struct arena *arena, size_t size)
{
	if (size == 0) {
		return NULL;
	}

	if (size > ARENA_CHUNK_SIZE) {
		return NULL;
	}

	struct arena_scope* scope = &arena->scopes[arena->scope_index];
	if (scope->data_offset + size > ARENA_CHUNK_SIZE)
	{
		// data does not fit, need new chunk
		if (scope->head->next == NULL)
		{
			// allocate new chunk
			int error = arena_alloc_chunk(arena);
			if (error != 0) {
				return NULL;
			}
		}

		scope->head = scope->head->next;
		scope->data_offset = 0;
	}

	void* result = &scope->head->data + scope->data_offset;
	scope->data_offset += size;
	return result;
}

void
arena_pop_scope(struct arena *arena)
{
	if (arena->scope_index <= 0) {
		return;
	}

	arena->scope_index--;
}

int
arena_alloc_chunk(struct arena *arena)
{
	struct arena_chunk* chunk = malloc(sizeof(struct arena_chunk));
	if (chunk == NULL) {
		return 1;
	}

	// init the chunk
	chunk->next = NULL;

	// append chunk
	arena->last->next = chunk;
	arena->last = chunk;

	return 0;
}

int
arena_count_chunks(struct arena* arena)
{
	int i = 1;
	struct arena_chunk* chunk = &arena->first;
	while (chunk->next != NULL)
	{
		chunk = chunk->next;
		i++;
	}
	return i;
}

void
arena_free(struct arena* arena)
{
	struct arena_chunk* chunk = arena->first.next;
	while (chunk != NULL)
	{
		struct arena_chunk* tofree = chunk;
		chunk = chunk->next;
		free(tofree);
	}

	free(arena);
}

void
arena_tests()
{
	int result;
	struct arena *arena = arena_init();

	assert(arena->scope_index == 0);
	assert(arena->scopes[arena->scope_index].data_offset == 0);
	assert(arena->scopes[arena->scope_index].head == &arena->first);
	assert(arena->last == &arena->first);

	void* alloc = arena_push(arena, 16);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 16);

	alloc = arena_push(arena, 16);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 32);

	alloc = arena_push(arena, 1024);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 1024);

	alloc = arena_push(arena, 32);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 32);

	alloc = arena_push(arena, 32);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 64);

	int data_offset = arena->scopes[arena->scope_index].data_offset;
	struct arena_chunk* head = arena->scopes[arena->scope_index].head;

	arena_push_scope(arena);

	alloc = arena_push(arena, 1000);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 1000);

	assert(arena_count_chunks(arena) == 4);

	arena_pop_scope(arena);

	assert(arena->scopes[arena->scope_index].data_offset == data_offset);
	assert(arena->scopes[arena->scope_index].head == head);

	assert(arena_count_chunks(arena) == 4);

	alloc = arena_push(arena, 1000);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 1000);

	assert(arena_count_chunks(arena) == 4);

	alloc = arena_push(arena, 1000);
	assert(alloc != NULL);
	assert(arena->scopes[arena->scope_index].data_offset == 1000);

	assert(arena_count_chunks(arena) == 5);

	arena_free(arena);

	printf("Tests done!\n");
}