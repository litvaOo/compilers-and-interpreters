#include "interpreter.h"
#include "lexer.h"
#include "memory.h"
#include "model.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
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

  Arena arena = new_arena();
  Lexer lexer = (Lexer){0, 0, 0, 1, 1, contents, file_size, &arena};
  tokenize(&lexer);

  // for (int i = 0; i < lexer.tokens_len; i++) {
  //   token_print(((Token *)arena.memory) + i);
  // }

  Parser parser = (Parser){0, lexer.tokens_len, &arena};
  Node new_expr = parse(&parser);

  // node_print(&new_expr);
  InterpretResult result = interpret_ast(new_expr, &arena);
  interpret_result_print(&result, "");
  // if (result.type == STR)
  //   free(result.String.value);
  free(contents);
  // free_statements(&new_expr.stmts);
  munmap(arena.memory, ARENA_SIZE);
  // // free(lexer.tokens);
  // // munmap(lexer.tokens, sizeof(Token) * lexer.tokens_size);
}
