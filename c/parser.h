#pragma once

#include "model.h"
#include "tokens.h"

typedef struct {
  int current;
  int tokens_list_len;
  Token *tokens_list;
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
Expression parse(Parser *self);
