#include "lexer.h"
#include "model.h"
#include "parser.h"
#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

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
  puts(contents);
  fclose(file);

  Lexer lexer = lexer_init(contents, file_size / sizeof(char));
  tokenize(&lexer);
  for (int i = 0; i < lexer.tokens_len - 1; i++) {
    token_print(&(lexer.tokens[i]));
  }

  Parser parser = (Parser){.current = 0,
                           .tokens_list_len = lexer.tokens_len,
                           .tokens_list = lexer.tokens};
  struct Expression new_expr = parse(&parser);
  expression_print(&new_expr);
}
