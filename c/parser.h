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
struct Expression *term(Parser *self);
struct Expression *expr(Parser *self);
struct Expression *primary(Parser *self);
struct Expression *unary(Parser *self);
struct Expression *factor(Parser *self);
struct Expression parse(Parser *self);
