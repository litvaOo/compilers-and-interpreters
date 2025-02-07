#include "model.h"
#include <stdio.h>
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

void statement_print(Statement *statement) {
  switch (statement->type) {
  case PRINT:
    printf("print ");
    expression_print(statement->PrintStatement.value);
    break;
  case PRINTLN:
    printf("println ");
    expression_print(statement->PrintlnStatement.value);
    break;
  case IF:
    printf("if ");
    expression_print(statement->IfStatement.test);
    break;
  case ASSIGNMENT:
    expression_print(statement->Assignment.left);
    printf(" = ");
    expression_print(statement->Assignment.right);
    break;
  case FOR:
    puts("for ");
    expression_print(statement->For.identifier);
    expression_print(statement->For.start);
    expression_print(statement->For.stop);
    expression_print(statement->For.step);
    print_statements(statement->For.stmts);
    puts("for end");
    break;
  case WHILE:
    puts("while ");
    expression_print(statement->While.test);
    print_statements(statement->While.stmts);
    puts("while end");
    break;
  }
  puts("");
}

void print_statements(Statements *stmts) {
  Statement *curr = stmts->head;
  while (curr != NULL) {
    statement_print(curr);
    curr = curr->next;
  }
}

void node_print(Node *node) {
  switch (node->type) {
  case EXPR:
    expression_print(node->expr);
    break;
  case STMT:
    statement_print(node->stmt);
    break;
  case STMTS:;
    print_statements(node->stmts);
    break;
  }
}
