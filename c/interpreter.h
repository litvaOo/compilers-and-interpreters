#pragma once

#include "model.h"
#include <stdbool.h>

typedef struct InterpretResult InterpretResult;

struct InterpretResult {
  enum RESULT_TYPE {
    BOOLEAN,
    NUMBER,
    STR,
  } type;
  struct {
    float value;
  } Number;
  struct {
    char *value;
  } String;
  struct {
    bool value;
  } Bool;
};

InterpretResult interpret(Expression *expression);
void interpret_result_print(InterpretResult *result);
