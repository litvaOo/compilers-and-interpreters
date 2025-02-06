#pragma once

#include "memory.h"
#include "model.h"
#include "tokens.h"

typedef struct {
  int current;
  int tokens_list_len;
  Arena *arena;
} Parser;

Token *advance_parser(Parser *self);
Token *expect(Parser *self, TokenType expected_type);
int is_next(Parser *self, TokenType expected_type);
Token peek_token(Parser *self);
int match_token(Parser *self, TokenType expected_type);
Token previous_token(Parser *self);
Expression *term(Parser *self);
Expression *expr(Parser *self);
Expression *primary(Parser *self);
Expression *unary(Parser *self);
Expression *logical_or(Parser *self);
Expression *factor(Parser *self);
Statement if_stmt(Parser *self);
Statement print_stmt(Parser *self);
Statement println_stmt(Parser *self);
Statement stmt(Parser *self);
Statements *stmts(Parser *self);
Node parse(Parser *self);
Parser init_parser(Token *tokens_list, unsigned int tokens_list_len);
Expression *push_expression(Parser *self, Expression expr);
