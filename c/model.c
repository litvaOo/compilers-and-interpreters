#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
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

// Statements init_statements(uint size) {
//   Statements arr = {mmap(NULL, sizeof(Statements) * 1024 * 1024 * 1024,
//                          PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
//                          0, 0),
//                     1024 * 1024 * 1024, 0};
//
//   return arr;
// }

// void push_item(Statements *arr, Statement statement) {
//   arr->statements[arr->length++] = statement;
// }

// void free_statements(Statements *arr) {
//   for (int i = 0; i < arr->length; i++) {
//     if (arr->statements[i].type == WHILE) {
//       free_statements(&(arr->statements[i].While.stmts));
//     }
//     if (arr->statements[i].type == IF) {
//       free_statements(&(arr->statements[i].IfStatement.else_stmts));
//       free_statements(&(arr->statements[i].IfStatement.then_stmts));
//     }
//     if (arr->statements[i].type == FOR) {
//       free_statements(&(arr->statements[i].For.stmts));
//     }
//   }
//   munmap(arr->statements, 1024 * 1024 * 1024);
// }
