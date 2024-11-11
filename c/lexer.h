#pragma once
#include "tokens.h"

typedef struct {
  long tokens_size;
  Token tokens[];
} Lexer;

Lexer lexer_init(char *source, long source_len);

void tokenize(Lexer lexers);
