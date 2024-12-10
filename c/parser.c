#include "parser.h"
#include "model.h"
#include "tokens.h"
#include <assert.h>
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

struct Expression *term(Parser *self) {
  struct Expression *express = factor(self);
  while (match_token(self, TokStar) || match_token(self, TokSlash)) {
    Token token = previous_token(self);
    struct Expression *fact = factor(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = fact;
    express = result;
  }
  return express;
}

struct Expression *expr(Parser *self) {
  struct Expression *express = term(self);
  while (match_token(self, TokPlus) || match_token(self, TokMinus)) {
    Token token = previous_token(self);
    struct Expression *term_ = term(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = term_;
    express = result;
  }
  return express;
}

struct Expression *primary(Parser *self) {
  if (match_token(self, TokInteger)) {
    Token token = previous_token(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = INTEGER;
    (*result).Integer.value = atoi(token.lexeme);
    return result;
  }
  if (match_token(self, TokInteger)) {
    Token token = previous_token(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = FLOAT;
    (*result).Float.value = atof(token.lexeme);
    return result;
  }
  if (match_token(self, TokLparen)) {
    struct Expression *express = term(self);
    {
      if (match_token(self, TokRparen)) {
        struct Expression *result = malloc(sizeof(struct Expression));
        (*result).type = GROUPING;
        (*result).Grouping.exp = express;
        return result;
      }
    }
  }
  assert("Shouldn't fails primary");
}

struct Expression *unary(Parser *self) {
  if (match_token(self, TokNot) || match_token(self, TokMinus) ||
      match_token(self, TokPlus)) {
    Token token = previous_token(self);
    struct Expression *express = unary(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = UNARY_OP;
    (*result).UnaryOp.op = token;
    (*result).UnaryOp.exp = express;
    return result;
  }
  return primary(self);
}

struct Expression *factor(Parser *self) { return unary(self); }

struct Expression parse(Parser *self) { return *(expr(self)); };
