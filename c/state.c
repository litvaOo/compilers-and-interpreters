#include "state.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
State state_new(State *parent) {
  State state = {calloc(10, sizeof(Variable)), 10, parent};
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
  if (hashed >= state->vars_size - 1) {
    unsigned int old_size = state->vars_size;
    state->vars_size = hashed + 12;
    state->vars = realloc(state->vars, state->vars_size * sizeof(Variable));
    memset(state->vars + old_size, 0, (hashed - old_size) * sizeof(Variable));
  }
  if (state->vars[hashed].set && state->vars[hashed].variable.type == STR &&
      state->vars[hashed].variable.String.alloced)
    free(state->vars[hashed].variable.String.value);
  state->vars[hashed] = (Variable){.variable = value, .set = true};
}

InterpretResult state_get(State *state, char *name, unsigned int len) {
  unsigned int hashed = hash_string(name, len);
  if ((hashed >= state->vars_size) || (!(state->vars[hashed].set))) {
    if (state->parent == NULL) {
      return (InterpretResult){.type = NONE};
    }
    return state_get(state->parent, name, len);
  }
  return state->vars[hashed].variable;
}

void free_state(State *state) {
  for (int i = 0; i < state->vars_size; i++) {
    if (state->vars[i].set) {
      if (state->vars[i].variable.type == STR &&
          state->vars[i].variable.String.alloced)
        free(state->vars[i].variable.String.value);
    }
  }
  free(state->vars);
}

State get_new_state(State *state) { return state_new(state); }
