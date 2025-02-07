#include "interpreter.h"
#include "memory.h"
#include "model.h"
#include "state.h"
#include "tokens.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InterpretResult interpret_ast(Node node, Arena *arena) {
  Arena hashmap_arena = new_arena();
  State state = state_new(NULL, &hashmap_arena);
  InterpretResult res = interpret(node, &state, arena, &hashmap_arena);
  free_state(&state, &hashmap_arena);
  return res;
}

InterpretResult interpret(Node node, State *state, Arena *arena,
                          Arena *hashmap_arena) {
  switch (node.type) {
  case EXPR:;
    Expression *expression = node.expr;
    InterpretResult left;
    InterpretResult right;
    switch (expression->type) {

    case (IDENTIFIER):;
      return state_get(state, expression->Identifier.name,
                       expression->Identifier.len);
    case (GROUPING):
      return interpret((Node){EXPR, .expr = expression->Grouping.exp}, state,
                       arena, hashmap_arena);
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
                               .String.value = expression->String.value,
                               .String.len = expression->String.len,
                               .String.alloced = false};

    case (UNARY_OP):
      right = interpret((Node){.type = EXPR, .expr = expression->UnaryOp.exp},
                        state, arena, hashmap_arena);
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
      left = interpret((Node){.type = EXPR, .expr = expression->LogicalOp.left},
                       state, arena, hashmap_arena);
      if (left.type == BOOLEAN) {
        if (expression->BinaryOp.op.token_type == TokOr &&
            left.Bool.value == true)
          return (InterpretResult){.type = BOOLEAN, .Bool.value = true};
        if (expression->BinaryOp.op.token_type == TokAnd &&
            left.Bool.value == false)
          return (InterpretResult){.type = BOOLEAN, .Bool.value = false};
        return interpret(
            (Node){.type = EXPR, .expr = expression->LogicalOp.right}, state,
            arena, hashmap_arena);
      }
      assert("Shouldn't reach here");
    case (BINARY_OP):
      left = interpret((Node){.type = EXPR, .expr = expression->BinaryOp.left},
                       state, arena, hashmap_arena);
      right =
          interpret((Node){.type = EXPR, .expr = expression->BinaryOp.right},
                    state, arena, hashmap_arena);
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
        if (expression->BinaryOp.op.token_type == TokPlus) {
          char *result =
              arena_alloc(arena, left.String.len + right.String.len + 1);
          strcpy(result, left.String.value);
          strcat(result, right.String.value);
          return (InterpretResult){.type = STR,
                                   .String.value = result,
                                   strlen(result),
                                   .String.alloced = true};
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
          char *result;
          if (right.Number.value == (int)right.Number.value) {
            result = arena_alloc(
                arena, left.String.len +
                           snprintf(NULL, 0, "%d", (int)right.Number.value) +
                           1);
            sprintf(result, "%.*s%d", left.String.len, left.String.value,
                    (int)right.Number.value);

          } else {
            result = arena_alloc(
                arena, left.String.len +
                           snprintf(NULL, 0, "%f", right.Number.value) + 1);
            sprintf(result, "%.*s%f", left.String.len, left.String.value,
                    right.Number.value);
          }
          return (InterpretResult){.type = STR,
                                   .String.value = result,
                                   .String.alloced = true,
                                   .String.len = strlen(result)};
        }
        if (expression->BinaryOp.op.token_type == TokStar) {
          char *result =
              arena_alloc(arena, left.String.len * (int)right.Number.value + 1);
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
  case STMTS:;
    Statement *current_stmt = node.stmts->head;
    while (current_stmt != NULL) {
      interpret((Node){.type = STMT, .stmt = current_stmt}, state, arena,
                hashmap_arena);
      current_stmt = current_stmt->next;
    };
    return (InterpretResult){.type = NONE};
  case STMT:;
    Statement *statement = node.stmt;
    InterpretResult res;
    switch (statement->type) {
    case PRINT:
      res = interpret(
          (Node){.type = EXPR, .expr = statement->PrintStatement.value}, state,
          arena, hashmap_arena);
      interpret_result_print(&res, "");
      break;
    case PRINTLN:
      res = interpret(
          (Node){.type = EXPR, .expr = statement->PrintlnStatement.value},
          state, arena, hashmap_arena);
      interpret_result_print(&res, "\n");
      break;
    case WHILE:;
      State new_state = get_new_state(state, hashmap_arena);
      while (1) {
        InterpretResult test_res =
            interpret((Node){.type = EXPR, .expr = statement->While.test},
                      &new_state, arena, hashmap_arena);
        bool stop = false;
        switch (test_res.type) {
        case BOOLEAN:
          if (!test_res.Bool.value)
            stop = true;
          break;
        case STR:
          if (test_res.String.len == 0)
            stop = true;
          break;
        case NUMBER:
          if (test_res.Number.value == 0.0)
            stop = true;
          break;
        case NONE:
          assert("shouldn't be here");
        }
        if (stop)
          break;
        interpret((Node){.type = STMTS, .stmts = statement->While.stmts},
                  &new_state, arena, hashmap_arena);
      }
      free_state(&new_state, hashmap_arena);
      break;
    case FOR:;
      State for_state = get_new_state(state, hashmap_arena);
      Expression *identifier = statement->For.identifier;
      InterpretResult start =
          interpret((Node){.type = EXPR, .expr = statement->For.start},
                    &for_state, arena, hashmap_arena);
      state_set(&for_state, identifier->Identifier.name,
                identifier->Identifier.len, start);
      InterpretResult stop =
          interpret((Node){.type = EXPR, .expr = statement->For.stop},
                    &for_state, arena, hashmap_arena);
      InterpretResult step =
          interpret((Node){.type = EXPR, .expr = statement->For.step},
                    &for_state, arena, hashmap_arena);
      while (1) {
        InterpretResult current_val =
            state_get(&for_state, identifier->Identifier.name,
                      identifier->Identifier.len);
        if (((start.Number.value <= stop.Number.value) &&
             (current_val.Number.value >= stop.Number.value)) ||
            ((start.Number.value >= stop.Number.value) &&
             (current_val.Number.value <= stop.Number.value))) {
          break;
        }
        interpret((Node){.type = STMTS, .stmts = statement->For.stmts},
                  &for_state, arena, hashmap_arena);
        current_val.Number.value += step.Number.value;
        state_set(&for_state, identifier->Identifier.name,
                  identifier->Identifier.len, current_val);
      }
      free_state(&for_state, hashmap_arena);
      break;
    case IF:
      res = interpret((Node){.type = EXPR, .expr = statement->IfStatement.test},
                      state, arena, hashmap_arena);
      State child_state = get_new_state(state, hashmap_arena);
      InterpretResult result;
      switch (res.type) {
      case NUMBER:
        if (res.Number.value == 0.0)
          result = interpret(
              (Node){.type = STMTS, .stmts = statement->IfStatement.then_stmts},
              &child_state, arena, hashmap_arena);
        else
          result = interpret(
              (Node){.type = STMTS, .stmts = statement->IfStatement.else_stmts},
              &child_state, arena, hashmap_arena);
        break;
      case BOOLEAN:
        if (res.Bool.value == true)
          result = interpret(
              (Node){.type = STMTS, .stmts = statement->IfStatement.then_stmts},
              &child_state, arena, hashmap_arena);
        else
          result = interpret(
              (Node){.type = STMTS, .stmts = statement->IfStatement.else_stmts},
              &child_state, arena, hashmap_arena);
        break;
      case STR:
        if (res.String.len != 0)
          result = interpret(
              (Node){.type = STMTS, .stmts = statement->IfStatement.then_stmts},
              &child_state, arena, hashmap_arena);
        else
          result = interpret(
              (Node){.type = STMTS, .stmts = statement->IfStatement.else_stmts},
              &child_state, arena, hashmap_arena);
        break;
      case NONE:
        assert("shouldn't be here");
      }
      assert("shouldn't be here");
      free_state(&child_state, hashmap_arena);
      return result;
      break;
    case ASSIGNMENT:;
      InterpretResult rres =
          interpret((Node){.type = EXPR, .expr = statement->Assignment.right},
                    state, arena, hashmap_arena);
      if (statement->Assignment.left->type == IDENTIFIER) {
        state_set(state, statement->Assignment.left->Identifier.name,
                  statement->Assignment.left->Identifier.len, rres);
        return (InterpretResult){.type = NONE};
      }
      assert("Tried to assign not to Identifier");
    }
    assert("Should know the type of Node");
    break;
  }
  assert("Shouldn't leave switch");
  return (InterpretResult){.type = NONE};
}

void interpret_result_print(InterpretResult *result, char *newline) {
  switch (result->type) {
  case (NUMBER):
    if (result->Number.value == (int)result->Number.value)
      printf("%d%s", (int)result->Number.value, newline);
    else
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
