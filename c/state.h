#pragma once

#include "memory.h"
#include "model.h"
typedef struct State State;
typedef struct Variable Variable;

struct State {
  Variable *vars;
  Statement **funcs;
  unsigned int vars_size;
  State *parent;
};

struct Variable {
  InterpretResult variable;
  bool set;
};

inline State state_new(State *parent, Arena *arena) {
  return (State){(Variable *)arena_alloc(arena, 2048 * sizeof(Variable)),
                 (Statement **)arena_alloc(arena, 2048 * sizeof(Statement *)),
                 2048, parent};
}
void state_set(State *state, char *name, unsigned int len,
               InterpretResult value);
InterpretResult state_get(State *state, char *name, unsigned int len);
inline void free_state(State *state, Arena *arena) {
  arena->pointer -= state->vars_size * sizeof(Variable);
}

inline State get_new_state(State *state, Arena *arena) {
  return state_new(state, arena);
}
