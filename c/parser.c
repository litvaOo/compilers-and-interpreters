#include "parser.h"
#include "memory.h"
#include "model.h"
#include "tokens.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>

Expression *push_expression(Parser *self, Expression expr) {
  Expression *new_expression = arena_alloc(self->arena, sizeof(Expression));
  *new_expression = expr;
  return new_expression;
}

Token *advance_parser(Parser *self) {
  if (self->current < self->tokens_list_len) {
    return ((Token *)self->arena->memory) + self->current++;
  }
  assert("Shouldn't request more then len token");
  return NULL;
}

int is_next(Parser *self, TokenType expected_type) {
  Token token = peek_token(self);
  if (token.token_type == expected_type)
    return 1;
  return 0;
}

Token *expect(Parser *self, TokenType expected_type) {
  Token token = peek_token(self);
  if (token.token_type == expected_type) {
    return advance_parser(self);
  };
  return NULL;
}

Token peek_token(Parser *self) {
  if (self->current < self->tokens_list_len) {
    return ((Token *)self->arena->memory)[self->current];
  }
  assert("Shouldn't request more then len token");
  return (Token){};
}

int match_token(Parser *self, TokenType expected_type) {
  Token token = peek_token(self);
  self->current += token.token_type == expected_type;
  return token.token_type == expected_type;
}

Token previous_token(Parser *self) {
  if (self->current > 0)
    return ((Token *)self->arena->memory)[self->current - 1];
  assert("Shouldn't request -1 token");
  return (Token){};
}

Expression *exponent(Parser *self) {
  Expression *express = factor(self);
  while (match_token(self, TokCaret)) {
    express = push_expression(
        self, (Expression){BINARY_OP, .BinaryOp = {previous_token(self),
                                                   express, exponent(self)}});
  }
  return express;
}

Expression *modulo(Parser *self) {
  Expression *express = exponent(self);
  while (match_token(self, TokMod)) {
    express = push_expression(
        self, (Expression){BINARY_OP, .BinaryOp = {previous_token(self),
                                                   express, factor(self)}});
  }
  return express;
}

Expression *term(Parser *self) {
  Expression *express = modulo(self);
  while (match_token(self, TokStar) || match_token(self, TokSlash)) {
    express = push_expression(
        self, (Expression){BINARY_OP, .BinaryOp = {previous_token(self),
                                                   express, modulo(self)}});
  }
  return express;
}

Expression *expr(Parser *self) {
  Expression *express = term(self);
  while (match_token(self, TokPlus) || match_token(self, TokMinus)) {
    express = push_expression(
        self, (Expression){BINARY_OP, .BinaryOp = {previous_token(self),
                                                   express, term(self)}});
  }
  return express;
}

Expression *primary(Parser *self) {
  if (match_token(self, TokInteger)) {
    Token token = previous_token(self);
    return push_expression(
        self,
        (Expression){INTEGER, .Integer = {strtol(token.lexeme, NULL, 10)}});
  }
  if (match_token(self, TokFloat)) {
    Token token = previous_token(self);
    return push_expression(
        self, (Expression){FLOAT, .Float = {strtof(token.lexeme, NULL)}});
  }
  if (match_token(self, TokTrue)) {
    return push_expression(self, (Expression){BOOL, .Bool = {1}});
  }
  if (match_token(self, TokFalse)) {
    return push_expression(self, (Expression){BOOL, .Bool = {0}});
  }
  if (match_token(self, TokString)) {
    Token token = previous_token(self);
    return push_expression(self, (Expression){STRING, .String = {
                                                          token.lexeme + 1,
                                                          token.lexeme_len - 2,
                                                      }});
  }
  if (match_token(self, TokLparen)) {
    Expression *express = logical_or(self);
    {
      if (match_token(self, TokRparen)) {
        return push_expression(self,
                               (Expression){GROUPING, .Grouping = {express}});
      }
    }
  }
  Token *token = expect(self, TokIdentifier);
  return push_expression(
      self, (Expression){.type = IDENTIFIER,
                         .Identifier = {.name = token->lexeme,
                                        .len = token->lexeme_len}});
}

Expression *unary(Parser *self) {
  if (match_token(self, TokNot) || match_token(self, TokMinus) ||
      match_token(self, TokPlus)) {
    return push_expression(
        self,
        (Expression){UNARY_OP, .UnaryOp = {previous_token(self), unary(self)}});
  }
  return primary(self);
}

Expression *factor(Parser *self) { return unary(self); }

Expression *compare(Parser *self) {
  Expression *express = expr(self);
  while (match_token(self, TokGt) || match_token(self, TokLt) ||
         match_token(self, TokGe) || match_token(self, TokLe)) {
    express = push_expression(
        self, (Expression){LOGICAL_OP, .LogicalOp = {previous_token(self),
                                                     express, expr(self)}});
  }
  return express;
}

Expression *equality(Parser *self) {
  Expression *express = compare(self);
  while (match_token(self, TokEq) || match_token(self, TokNe)) {
    express = push_expression(
        self, (Expression){LOGICAL_OP, .LogicalOp = {previous_token(self),
                                                     express, compare(self)}});
  }
  return express;
}

Expression *logical_and(Parser *self) {
  Expression *express = equality(self);
  while (match_token(self, TokAnd)) {
    express = push_expression(
        self, (Expression){LOGICAL_OP, .LogicalOp = {previous_token(self),
                                                     express, equality(self)}});
  }
  return express;
}

Expression *logical_or(Parser *self) {
  Expression *express = logical_and(self);
  while (match_token(self, TokOr)) {
    express = push_expression(
        self,
        (Expression){LOGICAL_OP, .LogicalOp = {previous_token(self), express,
                                               logical_and(self)}});
  }
  return express;
}

Statement if_stmt(Parser *self) {
  expect(self, TokIf);
  Expression *test = logical_or(self);
  expect(self, TokThen);
  Statements *then_stmts = stmts(self);
  Statements *else_stmts = arena_alloc(self->arena, sizeof(Statements));
  if (is_next(self, TokElse)) {
    advance_parser(self);
    else_stmts = stmts(self);
  }
  expect(self, TokEnd);
  return (Statement){.type = IF, .IfStatement = {test, then_stmts, else_stmts}};
}

Statement while_stmt(Parser *self) {
  expect(self, TokWhile);
  Expression *test = logical_or(self);
  expect(self, TokThen);
  Statements *while_stmts = stmts(self);
  expect(self, TokEnd);
  return (Statement){.type = WHILE,
                     .While = {.test = test, .stmts = while_stmts}};
}

Statement println_stmt(Parser *self) {
  if (match_token(self, TokPrintln)) {
    return (Statement){.type = PRINTLN,
                       .PrintlnStatement.value = logical_or(self)};
  }
  exit(10);
}

Statement print_stmt(Parser *self) {
  if (match_token(self, TokPrint)) {
    return (Statement){.type = PRINT, .PrintStatement.value = logical_or(self)};
  }
  exit(11);
}

Statement for_stmt(Parser *self) {
  expect(self, TokFor);
  Expression *identifier = primary(self);
  expect(self, TokAssign);
  Expression *start = logical_or(self);
  expect(self, TokComma);
  Expression *stop = logical_or(self);
  Expression *step =
      push_expression(self, (Expression){.type = INTEGER, .Integer = {1}});
  if (match_token(self, TokComma)) {
    step = expr(self);
  }
  expect(self, TokDo);
  Statements *for_stmts = stmts(self);
  expect(self, TokEnd);
  return (Statement){.type = FOR,
                     .For = {.identifier = identifier,
                             .start = start,
                             .stop = stop,
                             .step = step,
                             .stmts = for_stmts}};
}

Statement stmt(Parser *self) {
  Token tok = peek_token(self);
  switch (tok.token_type) {
  case TokPrint:
    return print_stmt(self);
  case TokPrintln:
    return println_stmt(self);
  case TokIf:
    return if_stmt(self);
  case TokWhile:
    return while_stmt(self);
  case TokFor:
    return for_stmt(self);
  default:;
    Expression *left = primary(self);
    if (match_token(self, TokAssign)) {
      Expression *right = expr(self);
      return (Statement){.type = ASSIGNMENT,
                         .Assignment = {.left = left, .right = right}};
    }
    assert("Unknown type of statemnt");
  }
  return (Statement){};
}

Statements *stmts(Parser *self) {
  Statement *curr = arena_alloc(self->arena, sizeof(Statement));
  Statements *stmts_arr = arena_alloc(self->arena, sizeof(Statements));
  stmts_arr->head = curr;
  while (true) {
    *curr = stmt(self);
    if (!((self->current < self->tokens_list_len - 1) &&
          (!is_next(self, TokElse)) && (!is_next(self, TokEnd))))
      break;
    curr->next = arena_alloc(self->arena, sizeof(Statement));
    curr = curr->next;
  };
  return stmts_arr;
}

Node parse(Parser *self) {
  return (Node){.type = STMTS, .stmts = stmts(self)};
};
