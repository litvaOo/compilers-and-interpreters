#include "parser.h"
#include "model.h"
#include "tokens.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

Token *advance_parser(Parser *self) {
  if (self->current < self->tokens_list_len - 1) {
    self->current++;
  }
  return &(self->tokens_list[self->current]);
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
    return self->tokens_list[self->current];
  }
  assert("Shouldn't request more then len token");
}

int match_token(Parser *self, TokenType expected_type) {
  Token token = peek_token(self);
  self->current += token.token_type == expected_type;
  return token.token_type == expected_type;
}

Token previous_token(Parser *self) {
  if (self->current > 0)
    return self->tokens_list[self->current - 1];
  assert("Shouldn't request -1 token");
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
    Token token = previous_token(self);
    return push_expression(self, (Expression){BOOL, .Bool = {1}});
  }
  if (match_token(self, TokFalse)) {
    Token token = previous_token(self);
    return push_expression(self, (Expression){BOOL, .Bool = {0}});
  }
  if (match_token(self, TokString)) {
    Token token = previous_token(self);
    return push_expression(
        self, (Expression){STRING,
                           .String = {token.lexeme + 1, token.lexeme_len - 2}});
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
  assert("Shouldn't fails primary");
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
  Statements then_stmts = stmts(self);
  Statements else_stmts = init_statements(4);
  if (is_next(self, TokElse)) {
    advance_parser(self);
    free(else_stmts.statements);
    else_stmts = stmts(self);
  }
  expect(self, TokEnd);
  return (Statement){.type = IF,
                     .IfStatement = {*test, then_stmts, else_stmts}};
}

Statement println_stmt(Parser *self) {
  if (match_token(self, TokPrintln)) {
    return (Statement){.type = PRINTLN,
                       .PrintlnStatement.value = *(logical_or(self))};
  }
  exit(10);
}

Statement print_stmt(Parser *self) {
  if (match_token(self, TokPrint)) {
    return (Statement){.type = PRINT,
                       .PrintStatement.value = *(logical_or(self))};
  }
  exit(11);
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
  default:
    exit(12);
  }
}

Statements stmts(Parser *self) {
  Statements stmts_arr = init_statements(64);
  while ((self->current < self->tokens_list_len - 1) &&
         (!is_next(self, TokElse)) && (!is_next(self, TokEnd)))
    push_item(&stmts_arr, stmt(self));
  return stmts_arr;
}

Node parse(Parser *self) { return (Node){.stmts = stmts(self)}; };
Parser init_parser(Token *tokens_list, unsigned int tokens_list_len) {
  Parser parser = (Parser){0, tokens_list_len, tokens_list};
  parser.expressions_arena = calloc(32, sizeof(Expression));
  parser.expressions_len = 0;
  parser.expressions_size = 32;
  return parser;
}

Expression *push_expression(Parser *self, Expression expr) {
  if (self->expressions_len + 1 == self->expressions_size) {
    puts("First realloc");
    self->expressions_size *= 2;
    self->expressions_arena = realloc(
        self->expressions_arena, (self->expressions_size) * sizeof(Expression));
  }
  self->expressions_arena[self->expressions_len++] = expr;
  return self->expressions_arena + self->expressions_len - 1;
}
