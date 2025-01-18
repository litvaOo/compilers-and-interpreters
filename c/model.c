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
    printf("%s ", expression->String.value);
    break;
  case (UNARY_OP):
    printf("(%s", expression->UnaryOp.op.lexeme);
    expression_print(expression->UnaryOp.exp);
    printf(")");
    break;
  case (BINARY_OP):
    printf("(%s", expression->BinaryOp.op.lexeme);
    expression_print(expression->BinaryOp.left);
    expression_print(expression->BinaryOp.right);
    printf(")");
    break;
  case (GROUPING):
    expression_print(expression->Grouping.exp);
    break;
  case (LOGICAL_OP):
    printf("(%s", expression->LogicalOp.op.lexeme);
    expression_print(expression->LogicalOp.left);
    expression_print(expression->LogicalOp.right);
    printf(")");
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
  arr->statements[arr->length] = statement;
  arr->length++;
}
