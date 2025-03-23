#pragma once

#include "tokens.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct InterpretResult InterpretResult;

struct InterpretResult {
  enum RESULT_TYPE { BOOLEAN, RETURN, NUMBER, STR, NONE } type;
  struct {
    float value;
  } Number;
  struct {
    char *value;
    int len;
    bool alloced;
  } String;
  struct {
    InterpretResult *ret;
  } Return;
  struct {
    bool value;
  } Bool;
};

enum EXPRESSION_TYPE {
  INTEGER,
  FLOAT,
  BOOL,
  STRING,
  UNARY_OP,
  LOGICAL_OP,
  BINARY_OP,
  GROUPING,
  IDENTIFIER,
  FUNCTION_CALL,
};

typedef struct Expression Expression;
typedef struct Expressions Expressions;

struct Expressions {
  Expression *head;
  int length;
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
      unsigned int len;
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
    struct {
      char *name;
      unsigned int len;
    } Identifier;
    struct {
      char *name;
      unsigned int name_len;
      Expressions *args;
    } FunctionCall;
  };
  Expression *next;
} __attribute__((aligned(8)));

enum STATEMENT_TYPE {
  PRINT,
  PRINTLN,
  IF,
  ASSIGNMENT,
  WHILE,
  FOR,
  PARAMETER,
  STATEMENT_FUNCTION_CALL,
  FUNCTION_DECLARATION,
  RET,
  LOCAL_ASSIGNMENT,
} __attribute__((aligned(8)));

typedef struct Statement Statement;
typedef struct Statements Statements;

struct Statements {
  Statement *head;
  int length;
};

struct Statement {
  enum STATEMENT_TYPE type;
  union {
    struct {
      Expression *value;
    } PrintStatement;
    struct {
      Expression *value;
    } PrintlnStatement;
    struct {
      Expression *test;
      Statements *then_stmts;
      Statements *else_stmts;
    } IfStatement;
    struct {
      Expression *left;
      Expression *right;
    } Assignment;
    struct {
      Expression *test;
      Statements *stmts;
    } While;
    struct {
      Expression *identifier;
      Expression *start;
      Expression *stop;
      Expression *step;
      Statements *stmts;
    } For;
    struct {
      char *name;
      unsigned int name_len;
    } Parameter;
    struct {
      Expression *expr;
    } FunctionCall;
    struct {
      char *name;
      unsigned int name_len;
      Statements *params;
      Statements *stmts;
    } __attribute__((aligned(8))) FunctionDeclaration;
    struct {
      Expression val;
    } Return;
    struct {
      Expression left;
      Expression right;
    } LocalAssignment;
  };
  Statement *next;
} __attribute__((aligned(8)));

enum NODE_TYPE {
  EXPR,
  STMT,
  STMTS,
};

typedef struct Node Node;

struct Node {
  enum NODE_TYPE type;
  union {
    Expression *expr;
    Statement *stmt;
    Statements *stmts;
  };
} __attribute__((aligned(8)));

void expression_print(struct Expression *expression);
Statements init_statements(uint size);
void push_item(Statements *arr, Statement statement);
void free_statements(Statements *arr);
void statement_print(Statement *statement);
void print_statements(Statements *stmts);
void node_print(Node *node);
