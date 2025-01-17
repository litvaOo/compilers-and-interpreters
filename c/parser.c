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
    Token token = previous_token(self);
    Expression *fact = exponent(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = fact;
    express = result;
  }
  return express;
}

Expression *modulo(Parser *self) {
  Expression *express = exponent(self);
  while (match_token(self, TokMod)) {
    Token token = previous_token(self);
    Expression *fact = factor(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = fact;
    express = result;
  }
  return express;
}

Expression *term(Parser *self) {
  Expression *express = modulo(self);
  while (match_token(self, TokStar) || match_token(self, TokSlash)) {
    Token token = previous_token(self);
    Expression *fact = modulo(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = fact;
    express = result;
  }
  return express;
}

Expression *expr(Parser *self) {
  Expression *express = term(self);
  while (match_token(self, TokPlus) || match_token(self, TokMinus)) {
    Token token = previous_token(self);
    Expression *term_ = term(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = term_;
    express = result;
  }
  return express;
}

Expression *primary(Parser *self) {
  if (match_token(self, TokInteger)) {
    Token token = previous_token(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = INTEGER;
    (*result).Integer.value = atoi(token.lexeme);
    return result;
  }
  if (match_token(self, TokInteger)) {
    Token token = previous_token(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = FLOAT;
    (*result).Float.value = atof(token.lexeme);
    return result;
  }
  if (match_token(self, TokTrue)) {
    Token token = previous_token(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BOOL;
    (*result).Bool.value = 1;
    return result;
  }
  if (match_token(self, TokFalse)) {
    Token token = previous_token(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BOOL;
    (*result).Bool.value = 0;
    return result;
  }
  if (match_token(self, TokString)) {
    Token token = previous_token(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = STRING;
    (*result).String.value = calloc((strlen(token.lexeme) - 2), sizeof(char));
    strncpy((*result).String.value, token.lexeme + 1, strlen(token.lexeme) - 2);
    return result;
  }
  if (match_token(self, TokLparen)) {
    Expression *express = logical_or(self);
    {
      if (match_token(self, TokRparen)) {
        Expression *result = malloc(sizeof(Expression));
        (*result).type = GROUPING;
        (*result).Grouping.exp = express;
        return result;
      }
    }
  }
  assert("Shouldn't fails primary");
}

Expression *unary(Parser *self) {
  if (match_token(self, TokNot) || match_token(self, TokMinus) ||
      match_token(self, TokPlus)) {
    Token token = previous_token(self);
    puts(token.lexeme);
    Expression *express = unary(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = UNARY_OP;
    (*result).UnaryOp.op = token;
    (*result).UnaryOp.exp = express;
    return result;
  }
  return primary(self);
}

Expression *factor(Parser *self) { return unary(self); }

Expression *compare(Parser *self) {
  Expression *express = expr(self);
  while (match_token(self, TokGt) || match_token(self, TokLt) ||
         match_token(self, TokGe) || match_token(self, TokLe)) {
    Token token = previous_token(self);
    Expression *right = expr(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = right;
    express = result;
  }
  return express;
}

Expression *equality(Parser *self) {
  Expression *express = compare(self);
  while (match_token(self, TokEq) || match_token(self, TokNe)) {
    Token token = previous_token(self);
    Expression *right = compare(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = BINARY_OP;
    (*result).BinaryOp.op = token;
    (*result).BinaryOp.left = express;
    (*result).BinaryOp.right = right;
    express = result;
  }
  return express;
}

Expression *logical_and(Parser *self) {
  Expression *express = equality(self);
  while (match_token(self, TokAnd)) {
    Token token = previous_token(self);
    Expression *right = equality(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = LOGICAL_OP;
    (*result).LogicalOp.op = token;
    (*result).LogicalOp.left = express;
    (*result).LogicalOp.right = right;
    express = result;
  }
  return express;
}

Expression *logical_or(Parser *self) {
  Expression *express = logical_and(self);
  while (match_token(self, TokOr)) {
    Token token = previous_token(self);
    Expression *right = logical_and(self);
    Expression *result = malloc(sizeof(Expression));
    (*result).type = LOGICAL_OP;
    (*result).LogicalOp.op = token;
    (*result).LogicalOp.left = express;
    (*result).LogicalOp.right = right;
    express = result;
  }
  return express;
}

Expression parse(Parser *self) { return *(logical_or(self)); };
