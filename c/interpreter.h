#pragma once

#include "model.h"
#include <stdbool.h>
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

struct InterpretResult interpret(struct Expression *expression);
void interpret_result_print(struct InterpretResult *result);
