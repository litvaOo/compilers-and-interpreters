#pragma once

#include "model.h"
#include "state.h"
#include <stdbool.h>
#include <string.h>

InterpretResult interpret_ast(Node node);
InterpretResult interpret(Node node, State *state);
void interpret_result_print(InterpretResult *result, char *newline);
