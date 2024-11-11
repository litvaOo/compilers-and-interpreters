#pragma once

typedef struct {
  char *token_type;
  char *lexeme;
} Token;

Token token_init(char *token_type, char *lexeme);
