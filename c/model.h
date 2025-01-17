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

typedef struct Expression Expression;

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
      Expression *exp;
    } UnaryOp;
    struct {
      Token op;
      Expression *left;
      Expression *right;
    } BinaryOp;
    struct {
      Expression *exp;
    } Grouping;
    struct {
      Token op;
      Expression *left;
      Expression *right;
    } LogicalOp;
  };
};

enum STATEMENT_TYPE {
  PRINT,
  PRINTLN,
  IF,
};

typedef struct Statement Statement;
typedef Statement *Statements;

struct Statement {
  enum STATEMENT_TYPE type;
  union {
    struct {
      Expression value;
    } PrintStatement;
    struct {
      Expression value;
    } PrintlnStatement;
    struct {
      Expression test;
      Statements then_stmts;
      Statements else_stmts;
    } IfStatement;
  };
};

void expression_print(struct Expression *expression);
