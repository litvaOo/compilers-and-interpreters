#pragma once

#include "model.h"
#include <stdbool.h>
#include <string.h>

typedef struct InterpretResult InterpretResult;

struct InterpretResult {
  enum RESULT_TYPE { BOOLEAN, NUMBER, STR, NONE } type;
  struct {
    float value;
  } Number;
  struct {
    char *value;
    int len;
  } String;
  struct {
    bool value;
  } Bool;
};

InterpretResult interpret(Node node);
void interpret_result_print(InterpretResult *result, char *newline);
