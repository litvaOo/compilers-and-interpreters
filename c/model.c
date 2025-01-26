#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

void expression_print(Expression *expression) {
  switch (expression->type) {
  case (INTEGER):
    printf("%d ", expression->Integer.value);
    break;
  case (FLOAT):
    printf("%.f ", expression->Float.value);
    break;
  case (BOOL):
    printf("%s ", expression->Bool.value ? "true" : "false");
    break;
  case (STRING):
    printf("%.*s ", expression->String.len, expression->String.value);
    break;
  case (UNARY_OP):
    printf("(%.*s", expression->UnaryOp.op.lexeme_len,
           expression->UnaryOp.op.lexeme);
    expression_print(expression->UnaryOp.exp);
    printf(")");
    break;
  case (BINARY_OP):
    printf("(%.*s", expression->BinaryOp.op.lexeme_len,
           expression->BinaryOp.op.lexeme);
    expression_print(expression->BinaryOp.left);
    expression_print(expression->BinaryOp.right);
    printf(")");
    break;
  case (GROUPING):
    expression_print(expression->Grouping.exp);
    break;
  case (LOGICAL_OP):
    printf("(%.*s", expression->LogicalOp.op.lexeme_len,
           expression->LogicalOp.op.lexeme);
    expression_print(expression->LogicalOp.left);
    expression_print(expression->LogicalOp.right);
    printf(")");
    break;
  case (IDENTIFIER):
    printf("%.*s ", expression->Identifier.len, expression->Identifier.name);
    break;
  }
}

Statements init_statements(uint size) {
  Statements arr = {malloc(sizeof(Statements) * size), size, 0};

  return arr;
}

void push_item(Statements *arr, Statement statement) {
  if (arr->length + 1 == arr->size) {
    puts("First realloc");
    arr->size *= 2;
    arr->statements = realloc(arr->statements, (arr->size) * sizeof(Statement));
  }
  arr->statements[arr->length++] = statement;
}

void free_statements(Statements *arr) {
  for (int i = 0; i < arr->length; i++) {
    if (arr->statements[i].type == WHILE) {
      free_statements(&(arr->statements[i].While.stmts));
    }
    if (arr->statements[i].type == IF) {
      free_statements(&(arr->statements[i].IfStatement.else_stmts));
      free_statements(&(arr->statements[i].IfStatement.then_stmts));
    }
  }
  free(arr->statements);
}
