#pragma once

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>

#define ARENA_SIZE 1024 * 1024 * 1024
typedef struct Arena Arena;

struct Arena {
  char *memory;
  size_t pointer;
};

Arena new_arena(void);
void *arena_alloc(Arena *arena, size_t size);
void *arena_alloc_aligned(Arena *arena, size_t size, size_t align);
