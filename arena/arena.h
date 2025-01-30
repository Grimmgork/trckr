#pragma once

#define ARENA_CHUNK_SIZE 1024
#define MAX_SCOPE_DEPTH 64

struct arena;

struct arena* arena_init();
void arena_push_scope(struct arena *arena);
void* arena_push(struct arena *arena, size_t size);
void arena_pop_scope(struct arena *arena);
void arena_free(struct arena* arena);

void arena_tests();
