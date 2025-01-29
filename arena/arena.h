#pragma once

#define ARENA_CHUNK_SIZE 1024
#define MAX_SCOPE_DEPTH 64


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

struct arena* arena_init();
void arena_push_scope(struct arena *arena);
void* arena_alloc(struct arena *arena, size_t size);
void arena_pop_scope(struct arena *arena);
void arena_free(struct arena* arena);

void arena_tests();



// 0 p
// 5 p