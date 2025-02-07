#include "memory.h"

Arena new_arena(void) {
  return (Arena){mmap(NULL, ARENA_SIZE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0),
                 0};
};

void *arena_alloc_aligned(Arena *arena, size_t size, size_t align) {
  size_t offset;
  if (align != 0) {
    offset = (size_t)(arena->memory + arena->pointer) % align;
    if (offset > 0) {
      arena->pointer = arena->pointer - offset + align;
    }
  } else {
    offset = 0;
  }

  void *ptr = &arena->memory[arena->pointer];
  arena->pointer += size;
  return ptr;
}
