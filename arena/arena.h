#pragma once

#define ARENA_CHUNK_SIZE 1024
#define MAX_SCOPE_DEPTH 64


struct arena_chunk {
	struct arena_chunk* next;
	int data_offset;
	char data[ARENA_CHUNK_SIZE];
};

struct arena_scope {
	struct arena_scope* parent;
	struct arena_chunk* head;
};

struct arena {
	struct arena_scope* scope; // top scope
	struct arena_chunk* last; // last allocated chunk
	struct arena_chunk first;
};

void arena_init(struct arena *arena);
int arena_push_scope(struct arena *arena);
void* arena_push(struct arena *arena, size_t size);
void arena_pop_scope(struct arena *arena);

void arena_tests();
