#include "interpreter.h"
#include "model.h"
#include "tokens.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InterpretResult interpret(Expression *expression) {
  InterpretResult left;
  InterpretResult right;
  switch (expression->type) {

  case (GROUPING):
    return interpret(expression->Grouping.exp);
  case (INTEGER):
    return (InterpretResult){.type = NUMBER,
                             .Number.value = expression->Integer.value};
  case (FLOAT):
    return (InterpretResult){.type = NUMBER,
                             .Number.value = expression->Float.value};
  case (BOOL):
    return (InterpretResult){.type = BOOLEAN,
                             .Bool.value = expression->Bool.value};
  case (STRING):
    return (InterpretResult){.type = STR,
                             .String.value = expression->String.value};

  case (UNARY_OP):
    right = interpret(expression->UnaryOp.exp);
    switch (right.type) {
    case (NUMBER):
      if (expression->UnaryOp.op.token_type == TokMinus) {
        return (InterpretResult){.type = NUMBER,
                                 .Number.value = -right.Number.value};
        if (expression->UnaryOp.op.token_type == TokPlus) {
          return (InterpretResult){.type = NUMBER,
                                   .Number.value = +right.Number.value};
        }
        assert("Shouldn't reach here");
      }
    case (BOOLEAN):
      if (expression->UnaryOp.op.token_type == TokNot) {
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = !right.Bool.value};
      }
    default:
      assert("shouldn't reach here");
    }
  case (LOGICAL_OP):
    left = interpret(expression->LogicalOp.left);
    if (left.type == BOOLEAN) {
      if (expression->BinaryOp.op.token_type == TokOr &&
          left.Bool.value == true)
        return (InterpretResult){.type = BOOLEAN, .Bool.value = true};
      if (expression->BinaryOp.op.token_type == TokAnd &&
          left.Bool.value == false)
        return (InterpretResult){.type = BOOLEAN, .Bool.value = false};
      InterpretResult result = interpret(expression->LogicalOp.right);
    }
    assert("Shouldn't reach here");
  case (BINARY_OP):
    left = interpret(expression->BinaryOp.left);
    right = interpret(expression->BinaryOp.right);
    if ((left.type == NUMBER || left.type == BOOLEAN) &&
        (right.type == NUMBER || right.type == BOOLEAN)) {
      float left_value = left.type == NUMBER ? left.Number.value
                         : left.Bool.value   ? 1
                                             : 0;
      float right_value = right.type == NUMBER ? right.Number.value
                          : right.Bool.value   ? 1
                                               : 0;
      switch (expression->BinaryOp.op.token_type) {
      case (TokPlus):
        return (InterpretResult){.type = NUMBER,
                                 .Number.value = left_value + right_value};
      case (TokMinus):
        return (InterpretResult){.type = NUMBER,
                                 .Number.value = left_value - right_value};
      case (TokStar):
        return (InterpretResult){.type = NUMBER,
                                 .Number.value = left_value * right_value};
      case (TokSlash):
        return (InterpretResult){.type = NUMBER,
                                 .Number.value = left_value / right_value};
      case (TokMod):
        return (InterpretResult){
            .type = NUMBER, .Number.value = (int)left_value % (int)right_value};
      case (TokCaret):
        return (InterpretResult){.type = NUMBER,
                                 .Number.value = pow(left_value, right_value)};
      case (TokEq):
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = left_value == right_value};
      case (TokLe):
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = left_value <= right_value};
      case (TokGe):
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = left_value >= right_value};
      case (TokLt):
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = left_value < right_value};
      case (TokGt):
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = left_value > right_value};
      case (TokNe):
        return (InterpretResult){.type = BOOLEAN,
                                 .Bool.value = left_value != right_value};
      default:
        assert("Shouldn't reach here");
      }
    }
    if (left.type == STR && right.type == STR) {
      if (expression->BinaryOp.op.token_type == TokPlus) {
        char *result =
            malloc(strlen(left.String.value) + strlen(right.String.value) + 1);
        strcpy(result, left.String.value);
        strcat(result, right.String.value);
        return (InterpretResult){.type = STR, .String.value = result};
      }
      if (expression->BinaryOp.op.token_type == TokEq) {
        return (InterpretResult){
            .type = BOOLEAN,
            .Bool.value = strcmp(left.String.value, right.String.value) == 0};
      }
      if (expression->BinaryOp.op.token_type == TokNe) {
        return (InterpretResult){
            .type = BOOLEAN,
            .Bool.value = strcmp(left.String.value, right.String.value) != 0};
      }
      assert("Shouldn't reach here");
    }
    if (left.type == STR && right.type == NUMBER) {
      if (expression->BinaryOp.op.token_type == TokPlus) {
        char *result =
            calloc(strlen(left.String.value) +
                       snprintf(NULL, 0, "%f", right.Number.value) + 1,
                   sizeof(char));
        sprintf(result, "%s%f", left.String.value, right.Number.value);
        return (InterpretResult){.type = STR, .String.value = result};
      }
      if (expression->BinaryOp.op.token_type == TokStar) {
        char *result =
            calloc(strlen(left.String.value) * (int)right.Number.value + 1,
                   sizeof(char));
        for (int i = 0; i < right.Number.value; i++) {
          strcat(result, left.String.value);
        }
        return (InterpretResult){.type = STR, .String.value = result};
      }
      assert("Shouldn't reach here");
    }

  default:
    assert("Shouldn't reach here");
  }
}

void interpret_result_print(InterpretResult *result) {
  switch (result->type) {
  case (NUMBER):
    printf("Result: %f\n", result->Number.value);
    break;
  case (BOOLEAN):
    printf("Result: %s\n", result->Bool.value ? "true" : "false");
    break;
  case (STR):
    printf("Result: %s\n", result->String.value);
    break;
  }
}
