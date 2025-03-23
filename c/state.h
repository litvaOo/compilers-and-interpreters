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

State state_new(State *parent, Arena *arena);
void state_set(State *state, char *name, unsigned int len,
               InterpretResult value);
InterpretResult state_get(State *state, char *name, unsigned int len);
void free_state(State *state, Arena *arena);
State get_new_state(State *state, Arena *arena);
void state_set_local(State *state, char *name, unsigned int len,
                     InterpretResult value);
void state_func_set(State *state, char *name, unsigned int name_len,
                    Statement *value);
Statement *state_func_get(State *state, char *name, unsigned int name_len);
