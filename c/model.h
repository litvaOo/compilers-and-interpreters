#pragma once

#include "tokens.h"
#include <stdbool.h>
enum EXPRESSION_TYPE {
  INTEGER,
  FLOAT,
  BOOL,
  STRING,
  UNARY_OP,
  LOGICAL_OP,
  BINARY_OP,
  GROUPING
};

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
      bool value;
    } Bool;
    struct {
      char *value;
    } String;
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
    struct {
      Token op;
      struct Expression *left;
      struct Expression *right;
    } LogicalOp;
  };
};

void expression_print(struct Expression *expression);
