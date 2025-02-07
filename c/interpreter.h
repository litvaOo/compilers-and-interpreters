#pragma once

#include "memory.h"
#include "model.h"
#include "state.h"
#include <stdbool.h>
#include <string.h>

InterpretResult interpret_ast(Node node, Arena *arena);
InterpretResult interpret(Node node, State *state, Arena *arena,
                          Arena *hashmap_arena);
void interpret_result_print(InterpretResult *result, char *newline);
