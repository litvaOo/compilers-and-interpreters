#pragma once
#include "tokens.h"

typedef struct {
  long tokens_size;
  long tokens_len;
  int start;
  int curr;
  int line;
  int line_position;
  char *source;
  long source_len;
  Token *tokens;
} Lexer;

Lexer lexer_init(char *source, long source_len);

void tokenize(Lexer *lexers);

char advance(Lexer *lexer);

void add_token(Lexer *lexer, TokenType token_type);

char peek(Lexer *lexer);

char lookahead(Lexer *lexer);

char match_char(Lexer *lexer, char expected);

void handle_number(Lexer *lexer);

void handle_string(Lexer *lexer, char quote);

void handle_identifier(Lexer *lexer);
