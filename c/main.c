#include "interpreter.h"
#include "lexer.h"
#include "model.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    puts("No input file");
    exit(EXIT_FAILURE);
  }
  char *filename = argv[1];
  FILE *file = fopen(filename, "r");
  struct stat st;

  if (fstat(fileno(file), &st) != 0) {
    puts("Failed to get file stat");
    exit(EXIT_FAILURE);
  }

  long file_size = st.st_size;
  char *contents = malloc(file_size);
  fread(contents, 1, file_size, file);
  fclose(file);

  Lexer lexer = lexer_init(contents, file_size / sizeof(char));
  tokenize(&lexer);

  Parser parser = init_parser(lexer.tokens, lexer.tokens_len);
  Node new_expr = parse(&parser);

  InterpretResult result = interpret_ast(new_expr);
  interpret_result_print(&result, "");
  if (result.type == STR)
    free(result.String.value);
  munmap(parser.expressions_arena, 1024*1024);
  free(contents);
  free_statements(&new_expr.stmts);
  // free(lexer.tokens);
  munmap(lexer.tokens, sizeof(Token) * lexer.tokens_size);
}
