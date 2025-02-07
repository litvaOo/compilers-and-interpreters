#include "state.h"
#include "memory.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

State state_new(State *parent, Arena *arena) {
  State state = {arena_alloc(arena, 2048 * sizeof(Variable)), 2048, parent};
  return state;
}

unsigned int hash_string(char *name, unsigned int len) {
  unsigned int res = 0;
  for (int i = 0; i < len; i++)
    res += i * 10 + name[i];
  return res;
}

void state_set(State *state, char *name, unsigned int len,
               InterpretResult value) {
  if (state->parent != NULL &&
      state_get(state->parent, name, len).type != NONE) {
    state_set(state->parent, name, len, value);
    return;
  }
  unsigned int hashed = hash_string(name, len);
  state->vars[hashed] = (Variable){.variable = value, .set = true};
}

InterpretResult state_get(State *state, char *name, unsigned int len) {
  unsigned int hashed = hash_string(name, len);
  if (!(state->vars[hashed].set)) {
    if (state->parent == NULL) {
      return (InterpretResult){.type = NONE};
    }
    return state_get(state->parent, name, len);
  }
  return state->vars[hashed].variable;
}

void free_state(State *state, Arena *arena) {
  arena->pointer -= state->vars_size * sizeof(Variable);
}

State get_new_state(State *state, Arena *arena) {
  return state_new(state, arena);
}
