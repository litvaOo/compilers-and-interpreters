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

struct Expression *exponent(Parser *self) {
  struct Expression *express = factor(self);
  while (match_token(self, TokCaret)) {
    Token token = previous_token(self);
    struct Expression *fact = exponent(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = fact;
    express = result;
  }
  return express;
}

struct Expression *modulo(Parser *self) {
  struct Expression *express = exponent(self);
  while (match_token(self, TokMod)) {
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

struct Expression *term(Parser *self) {
  struct Expression *express = modulo(self);
  while (match_token(self, TokStar) || match_token(self, TokSlash)) {
    Token token = previous_token(self);
    struct Expression *fact = modulo(self);
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
  if (match_token(self, TokTrue)) {
    Token token = previous_token(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BOOL;
    (*result).Bool.value = 1;
    return result;
  }
  if (match_token(self, TokFalse)) {
    Token token = previous_token(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BOOL;
    (*result).Bool.value = 0;
    return result;
  }
  if (match_token(self, TokString)) {
    Token token = previous_token(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = STRING;
    (*result).String.value = calloc((strlen(token.lexeme) - 2), sizeof(char));
    strncpy((*result).String.value, token.lexeme + 1, strlen(token.lexeme) - 2);
    return result;
  }
  if (match_token(self, TokLparen)) {
    struct Expression *express = logical_or(self);
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
    puts(token.lexeme);
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

struct Expression *compare(Parser *self) {
  struct Expression *express = expr(self);
  while (match_token(self, TokGt) || match_token(self, TokLt) ||
         match_token(self, TokGe) || match_token(self, TokLe)) {
    Token token = previous_token(self);
    struct Expression *right = expr(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = right;
    express = result;
  }
  return express;
}

struct Expression *equality(Parser *self) {
  struct Expression *express = compare(self);
  while (match_token(self, TokEq) || match_token(self, TokNe)) {
    Token token = previous_token(self);
    struct Expression *right = compare(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = right;
    express = result;
  }
  return express;
}

struct Expression *logical_and(Parser *self) {
  struct Expression *express = equality(self);
  while (match_token(self, TokAnd)) {
    Token token = previous_token(self);
    struct Expression *right = equality(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = LOGICAL_OP;
    (*result).LogicalOp.op = token;
    (*result).LogicalOp.left = express;
    (*result).LogicalOp.right = right;
    express = result;
  }
  return express;
}

struct Expression *logical_or(Parser *self) {
  struct Expression *express = logical_and(self);
  while (match_token(self, TokOr)) {
    Token token = previous_token(self);
    struct Expression *right = logical_and(self);
    struct Expression *result = malloc(sizeof(struct Expression));
    (*result).type = LOGICAL_OP;
    (*result).LogicalOp.op = token;
    (*result).LogicalOp.left = express;
    (*result).LogicalOp.right = right;
    express = result;
  }
  return express;
}

struct Expression parse(Parser *self) { return *(logical_or(self)); };
