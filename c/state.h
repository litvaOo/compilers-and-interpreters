#pragma once

#include "model.h"
typedef struct State State;
typedef struct Variable Variable;

struct State {
  Variable *vars;
  unsigned int vars_size;
  State *parent;
};

struct Variable {
  InterpretResult variable;
  bool set;
};

State state_new(State *parent);
void state_set(State *state, char *name, unsigned int len,
               InterpretResult value);
InterpretResult state_get(State *state, char *name, unsigned int len);
State get_new_state(State *state);
