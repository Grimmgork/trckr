#include <stdio.h>   
#include <stdlib.h>
#include <assert.h>

#include <arena.h>

int arena_alloc_chunk(struct arena *arena);

void
arena_init(struct arena *arena)
{
	arena->scope = NULL;
	arena->last = &arena->first;
}

int
arena_push_scope(struct arena *arena)
{
	if (arena->scope == NULL)
	{
		struct arena_scope *scope = (void*)&(&arena->first)->data[0];
		scope->parent = NULL;
		scope->head = &arena->first;
		arena->scope = scope;

		arena->first.data_offset += sizeof(struct arena_scope);
		return 0;
	}
	else
	{
		struct arena_scope* scope = arena_push(arena, sizeof(struct arena_scope));
		if (scope == NULL) {
			return 1;
		}

		scope->parent = arena->scope;
		scope->head = scope->parent->head;
		arena->scope = scope;
		return 0;
	}
}

void*
arena_push(struct arena *arena, size_t size)
{
	if (size > ARENA_CHUNK_SIZE) {
		return NULL;
	}

	if (arena->scope == NULL) {
		return NULL;
	}

	struct arena_chunk* chunk = arena->scope->head;
	if (ARENA_CHUNK_SIZE - chunk->data_offset < size)
	{
		// not enough space
		int result = arena_alloc_chunk(arena);
		if (result != 0) {
			return NULL;
		}

		return &chunk->data;
	}
	else
	{
		// enough space
		void* ptr = &chunk->data[chunk->data_offset];
		chunk->data_offset += size;
		return ptr;
	}
}

void
arena_pop_scope(struct arena *arena)
{
	if (arena->scope == NULL) {
		return;
	}

	arena->scope = arena->scope->parent;
}

int
arena_alloc_chunk(struct arena *arena)
{
	struct arena_chunk *chunk = malloc(sizeof(struct arena_chunk));
	if (chunk == NULL) {
		return 1;
	}

	chunk->next = NULL;
	arena->last->next = chunk;
	arena->last = chunk;
	return 0;
}

void
arena_tests()
{
	int result;

	printf("start arena_tests\n");

	printf("init arena\n");
	struct arena arena;
	arena_init(&arena);

	assert(arena.last == &arena.first);

	printf("start scope 1\n");
	result = arena_push_scope(&arena);
	assert(result == 0);

	assert(arena.scope != NULL);
	assert(arena.scope->head == &arena.first);

	printf("start scope 2\n");
	result = arena_push_scope(&arena);
	assert(result == 0);

	assert(arena.scope != NULL);
	assert(arena.scope->parent != NULL);
	assert(arena.scope->head->data_offset == sizeof(struct arena_scope));

	printf("done!\n");
}