#pragma once

#include "tokens.h"
enum EXPRESSION_TYPE { INTEGER, FLOAT, UNARY_OP, BINARY_OP, GROUPING };

struct Expression {
  enum EXPRESSION_TYPE type;
  union {
    struct {
      int value;
    } Integer;
    struct {
      float value;
    } Float;
    struct {
      Token op;
      struct Expression *exp;
    } UnaryOp;
    struct {
      Token op;
      struct Expression *left;
      struct Expression *right;
    } BinaryOp;
    struct {
      struct Expression *exp;
    } Grouping;
  };
};

void expression_print(struct Expression *expression);
