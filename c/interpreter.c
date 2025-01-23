#include "interpreter.h"
#include "model.h"
#include "state.h"
#include "tokens.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InterpretResult interpret_ast(Node node) {
  State state = state_new(NULL);
  InterpretResult res = interpret(node, &state);
  puts("got res");
  free(state.vars);
  return res;
}

InterpretResult interpret(Node node, State *state) {
  switch (node.type) {
  case EXPR:;
    Expression expression = node.expr;
    InterpretResult left;
    InterpretResult right;
    switch (expression.type) {

    case (IDENTIFIER):;
      return state_get(state, expression.Identifier.name,
                       expression.Identifier.len);
    case (GROUPING):
      return interpret((Node){EXPR, .expr = *(expression.Grouping.exp)}, state);
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
                               .String.value = expression.String.value,
                               .String.len = expression.String.len,
                               .String.alloced = false};

    case (UNARY_OP):
      right = interpret((Node){.type = EXPR, .expr = expression}, state);
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
      left = interpret((Node){.type = EXPR, .expr = *expression.LogicalOp.left},
                       state);
      if (left.type == BOOLEAN) {
        if (expression.BinaryOp.op.token_type == TokOr &&
            left.Bool.value == true)
          return (InterpretResult){.type = BOOLEAN, .Bool.value = true};
        if (expression.BinaryOp.op.token_type == TokAnd &&
            left.Bool.value == false)
          return (InterpretResult){.type = BOOLEAN, .Bool.value = false};
        InterpretResult result = interpret(
            (Node){.type = EXPR, .expr = *expression.LogicalOp.right}, state);
      }
      assert("Shouldn't reach here");
    case (BINARY_OP):
      left = interpret((Node){.type = EXPR, .expr = *expression.BinaryOp.left},
                       state);
      right = interpret(
          (Node){.type = EXPR, .expr = *expression.BinaryOp.right}, state);
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
          char *result = malloc(left.String.len + right.String.len + 1);
          strcpy(result, left.String.value);
          strcat(result, right.String.value);
          return (InterpretResult){.type = STR,
                                   .String.value = result,
                                   strlen(result),
                                   .String.alloced = true};
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
          char *result = calloc(
              left.String.len + snprintf(NULL, 0, "%f", right.Number.value) + 1,
              sizeof(char));
          sprintf(result, "%.*s%f", left.String.len, left.String.value,
                  right.Number.value);
          return (InterpretResult){
              .type = STR, .String.value = result, .String.alloced = true};
        }
        if (expression.BinaryOp.op.token_type == TokStar) {
          char *result = calloc(left.String.len * (int)right.Number.value + 1,
                                sizeof(char));
          for (int i = 0; i < right.Number.value; i++) {
            strncat(result, left.String.value, left.String.len);
          }
          return (InterpretResult){.type = STR,
                                   .String.value = result,
                                   .String.len = strlen(result),
                                   .String.alloced = true};
        }
        assert("Shouldn't reach here");
      }

    default:
      assert("Shouldn't reach here");
    }
  case STMTS:
    for (int i = 0; i < node.stmts.length; i++)
      interpret((Node){.type = STMT, .stmt = node.stmts.statements[i]}, state);
    return (InterpretResult){.type = NONE};
  case STMT:;
    Statement statement = node.stmt;
    InterpretResult res;
    switch (statement.type) {
    case PRINT:
      res = interpret(
          (Node){.type = EXPR, .expr = statement.PrintStatement.value}, state);
      interpret_result_print(&res, "");
      if ((res.type == STR) && (res.String.alloced))
        free(res.String.value);
      break;
    case PRINTLN:
      res = interpret(
          (Node){.type = EXPR, .expr = statement.PrintlnStatement.value},
          state);
      interpret_result_print(&res, "\n");
      if ((res.type == STR) && (res.String.alloced))
        free(res.String.value);
      break;
    case IF:
      res = interpret((Node){.type = EXPR, .expr = statement.IfStatement.test},
                      state);
      State child_state = get_new_state(state);
      InterpretResult result;
      switch (res.type) {
      case NUMBER:
        if (res.Number.value == 0.0)
          result = interpret(
              (Node){.type = STMTS, .stmts = statement.IfStatement.then_stmts},
              &child_state);
        result = interpret(
            (Node){.type = STMTS, .stmts = statement.IfStatement.else_stmts},
            &child_state);
        break;
      case BOOLEAN:
        if (res.Bool.value == true)
          result = interpret(
              (Node){.type = STMTS, .stmts = statement.IfStatement.then_stmts},
              &child_state);
        result = interpret(
            (Node){.type = STMTS, .stmts = statement.IfStatement.else_stmts},
            &child_state);
        break;
      case STR:
        if (res.String.len != 0)
          result = interpret(
              (Node){.type = STMTS, .stmts = statement.IfStatement.then_stmts},
              &child_state);
        result = interpret(
            (Node){.type = STMTS, .stmts = statement.IfStatement.else_stmts},
            &child_state);
        break;
      }
      free(child_state.vars);
      return result;
      break;
    case ASSIGNMENT:;
      InterpretResult rres = interpret(
          (Node){.type = EXPR, .expr = statement.Assignment.right}, state);
      if (statement.Assignment.left.type == IDENTIFIER) {
        state_set(state, statement.Assignment.left.Identifier.name,
                  statement.Assignment.left.Identifier.len, rres);
        return (InterpretResult){.type = NONE};
      }
      assert("Tried to assign not to Identifier");
    }
    assert("Should know the type of Node");
    break;
  }
}

void interpret_result_print(InterpretResult *result, char *newline) {
  switch (result->type) {
  case (NUMBER):
    printf("%f%s", result->Number.value, newline);
    break;
  case (BOOLEAN):
    printf("%s%s", result->Bool.value ? "true" : "false", newline);
    break;
  case (STR):
    printf("%.*s%s", result->String.len, result->String.value, newline);
    break;
  case (NONE):
    break;
  }
}
