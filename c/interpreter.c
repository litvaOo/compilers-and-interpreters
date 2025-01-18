#include "interpreter.h"
#include "model.h"
#include "tokens.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InterpretResult interpret(Node node) {
  switch (node.type) {
  case EXPR:;
    Expression expression = node.expr;
    InterpretResult left;
    InterpretResult right;
    switch (expression.type) {

    case (GROUPING):
      return interpret((Node){EXPR, .expr = expression});
    case (INTEGER):
      return (InterpretResult){.type = NUMBER,
                               .Number.value = expression.Integer.value};
    case (FLOAT):
      return (InterpretResult){.type = NUMBER,
                               .Number.value = expression.Float.value};
    case (BOOL):
      return (InterpretResult){.type = BOOLEAN,
                               .Bool.value = expression.Bool.value};
    case (STRING):
      return (InterpretResult){.type = STR,
                               .String.value = expression.String.value};

    case (UNARY_OP):
      right = interpret((Node){.type = EXPR, .expr = expression});
      switch (right.type) {
      case (NUMBER):
        if (expression.UnaryOp.op.token_type == TokMinus) {
          return (InterpretResult){.type = NUMBER,
                                   .Number.value = -right.Number.value};
          if (expression.UnaryOp.op.token_type == TokPlus) {
            return (InterpretResult){.type = NUMBER,
                                     .Number.value = +right.Number.value};
          }
          assert("Shouldn't reach here");
        }
      case (BOOLEAN):
        if (expression.UnaryOp.op.token_type == TokNot) {
          return (InterpretResult){.type = BOOLEAN,
                                   .Bool.value = !right.Bool.value};
        }
      default:
        assert("shouldn't reach here");
      }
    case (LOGICAL_OP):
      left = interpret((Node){.type = EXPR, .expr = expression});
      if (left.type == BOOLEAN) {
        if (expression.BinaryOp.op.token_type == TokOr &&
            left.Bool.value == true)
          return (InterpretResult){.type = BOOLEAN, .Bool.value = true};
        if (expression.BinaryOp.op.token_type == TokAnd &&
            left.Bool.value == false)
          return (InterpretResult){.type = BOOLEAN, .Bool.value = false};
        InterpretResult result =
            interpret((Node){.type = EXPR, .expr = expression});
      }
      assert("Shouldn't reach here");
    case (BINARY_OP):
      left = interpret((Node){.type = EXPR, .expr = expression});
      right = interpret((Node){.type = EXPR, .expr = expression});
      if ((left.type == NUMBER || left.type == BOOLEAN) &&
          (right.type == NUMBER || right.type == BOOLEAN)) {
        float left_value = left.type == NUMBER ? left.Number.value
                           : left.Bool.value   ? 1
                                               : 0;
        float right_value = right.type == NUMBER ? right.Number.value
                            : right.Bool.value   ? 1
                                                 : 0;
        switch (expression.BinaryOp.op.token_type) {
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
          return (InterpretResult){.type = NUMBER,
                                   .Number.value =
                                       (int)left_value % (int)right_value};
        case (TokCaret):
          return (InterpretResult){
              .type = NUMBER, .Number.value = pow(left_value, right_value)};
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
        if (expression.BinaryOp.op.token_type == TokPlus) {
          char *result = malloc(strlen(left.String.value) +
                                strlen(right.String.value) + 1);
          strcpy(result, left.String.value);
          strcat(result, right.String.value);
          return (InterpretResult){.type = STR, .String.value = result};
        }
        if (expression.BinaryOp.op.token_type == TokEq) {
          return (InterpretResult){
              .type = BOOLEAN,
              .Bool.value = strcmp(left.String.value, right.String.value) == 0};
        }
        if (expression.BinaryOp.op.token_type == TokNe) {
          return (InterpretResult){
              .type = BOOLEAN,
              .Bool.value = strcmp(left.String.value, right.String.value) != 0};
        }
        assert("Shouldn't reach here");
      }
      if (left.type == STR && right.type == NUMBER) {
        if (expression.BinaryOp.op.token_type == TokPlus) {
          char *result =
              calloc(strlen(left.String.value) +
                         snprintf(NULL, 0, "%f", right.Number.value) + 1,
                     sizeof(char));
          sprintf(result, "%s%f", left.String.value, right.Number.value);
          return (InterpretResult){.type = STR, .String.value = result};
        }
        if (expression.BinaryOp.op.token_type == TokStar) {
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
  case STMTS:
    for (int i = 0; i < node.stmts.length; i++)
      interpret((Node){.type = STMT, .stmt = node.stmts.statements[i]});
    return (InterpretResult){.type = NONE};
  case STMT:;
    Statement statement = node.stmt;
    InterpretResult res;
    switch (statement.type) {
    case PRINT:
      res = interpret(
          (Node){.type = EXPR, .expr = statement.PrintStatement.value});
      interpret_result_print(&res);
      break;
    case PRINTLN:
      res = interpret(
          (Node){.type = EXPR, .expr = statement.PrintlnStatement.value});
      interpret_result_print(&res);
      break;
    case IF:
      res = interpret((Node){.type = EXPR, .expr = statement.IfStatement.test});
      switch (res.type) {
      case NUMBER:
        if (res.Number.value == 0.0)
          return interpret(
              (Node){.type = STMTS, .stmts = statement.IfStatement.then_stmts});
        return interpret(
            (Node){.type = STMTS, .stmts = statement.IfStatement.else_stmts});
      case BOOLEAN:
        if (res.Bool.value == false)
          return interpret(
              (Node){.type = STMTS, .stmts = statement.IfStatement.then_stmts});
        return interpret(
            (Node){.type = STMTS, .stmts = statement.IfStatement.else_stmts});
      case STR:
        if (strlen(res.String.value) != 0)
          return interpret(
              (Node){.type = STMTS, .stmts = statement.IfStatement.then_stmts});
        return interpret(
            (Node){.type = STMTS, .stmts = statement.IfStatement.else_stmts});
      case NONE:
        assert("Shouldn't get none as result");
      }
      break;
    }
    assert("Should know the type of Node");
    break;
  }
}

void interpret_result_print(InterpretResult *result) {
  switch (result->type) {
  case (NUMBER):
    printf("%f", result->Number.value);
    break;
  case (BOOLEAN):
    printf("%s", result->Bool.value ? "true" : "false");
    break;
  case (STR):
    printf("%s", result->String.value);
    break;
  case (NONE):
    break;
  }
}
