#include "state.h"
#include "model.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

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

void state_func_set(State *state, char *name, unsigned int name_len,
                    Statement *value) {
  unsigned int hashed = hash_string(name, name_len);
  state->funcs[hashed] = value;
}

Statement *state_func_get(State *state, char *name, unsigned int name_len) {
  unsigned int hashed = hash_string(name, name_len);
  return *(state->funcs + hashed);
}

void state_set_local(State *state, char *name, unsigned int len,
                     InterpretResult value) {
  unsigned int hashed = hash_string(name, len);
  state->vars[hashed] = (Variable){.variable = value, .set = true};
}

void free_state(State *state, Arena *arena) {
  arena->pointer -= state->vars_size * sizeof(Variable);
}

State get_new_state(State *state, Arena *arena) {
  return state_new(state, arena);
}
State state_new(State *parent, Arena *arena) {
  return (State){(Variable *)arena_alloc(arena, 4098 * sizeof(Variable)),
                 (Statement **)arena_alloc(arena, 4098 * sizeof(Statement *)),
                 2048, parent};
}
