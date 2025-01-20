#include "lexer.h"
#include "tokens.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

Lexer lexer_init(char *source, long source_len) {
  Lexer lexer = (Lexer){
      4, 1, 0, 0, 1, 1, source, source_len,
  };
  lexer.tokens = (Token *)malloc(sizeof(Token) * 4);
  return lexer;
}

char advance(Lexer *lexer) {
  if (lexer->curr >= lexer->source_len)
    return '\0';
  char ch = lexer->source[lexer->curr];
  lexer->curr++;
  lexer->line_position++;
  return ch;
}

void add_token(Lexer *lexer, TokenType token_type) {
  if (lexer->tokens_len == lexer->tokens_size) {
    lexer->tokens_size *= 2;
    lexer->tokens = realloc(lexer->tokens, lexer->tokens_size * sizeof(Token));
  }
  Token token =
      token_init(token_type, &lexer->source[lexer->start], lexer->line,
                 lexer->curr - lexer->start, lexer->line_position);
  lexer->tokens[lexer->tokens_len - 1] = token;
  lexer->tokens_len++;
}

char peek(Lexer *lexer) {
  if (lexer->curr >= lexer->source_len)
    return '\0';
  return lexer->source[lexer->curr];
}

char lookahead(Lexer *lexer) {
  if (lexer->curr + 1 >= lexer->source_len)
    return '\0';
  return lexer->source[lexer->curr + 1];
}

char match_char(Lexer *lexer, char expected) {
  if ((lexer->curr >= lexer->source_len) ||
      lexer->source[lexer->curr] != expected)
    return 1;
  lexer->curr++;
  return 0;
}

void handle_number(Lexer *lexer) {
  while (isdigit(peek(lexer)))
    advance(lexer);
  if ((peek(lexer) == '.') && (isdigit(lookahead(lexer)))) {
    advance(lexer);
    while (isdigit(peek(lexer)))
      advance(lexer);
    add_token(lexer, TokFloat);
  } else {
    add_token(lexer, TokInteger);
  }
}

void handle_string(Lexer *lexer, char quote) {
  while ((lexer->curr < lexer->source_len) &&
         (lexer->source[lexer->curr] != quote))
    advance(lexer);
  if (lexer->curr >= lexer->source_len) {
    printf("Unterminated string starting at line %d at position %d",
           lexer->line, lexer->line_position);
    exit(EXIT_FAILURE);
  }
  advance(lexer);
  add_token(lexer, TokString);
}

void handle_identifier(Lexer *lexer) {
  while ((isalnum(peek(lexer))) || (peek(lexer) == '_'))
    advance(lexer);
  add_token(lexer,
            keywords(&lexer->source[lexer->start], lexer->curr - lexer->start));
}

void tokenize(Lexer *lexer) {
  while (lexer->curr < lexer->source_len) {
    lexer->start = lexer->curr;
    char ch = advance(lexer);
    switch (ch) {
    case '\n':
      lexer->line++;
      lexer->line_position = 1;
      break;
    case '#':
      while ((peek(lexer) != '\n') && (lexer->curr < lexer->source_len))
        advance(lexer);
      break;
    case '(':
      add_token(lexer, TokLparen);
      break;
    case ')':
      add_token(lexer, TokRparen);
      break;
    case '{':
      add_token(lexer, TokLcurly);
      break;
    case '}':
      add_token(lexer, TokRcurly);
      break;
    case '[':
      add_token(lexer, TokLsquar);
      break;
    case ']':
      add_token(lexer, TokRsquar);
      break;
    case ',':
      add_token(lexer, TokComma);
      break;
    case '.':
      add_token(lexer, TokDot);
      break;
    case '+':
      add_token(lexer, TokPlus);
      break;
    case '-':
      if (match_char(lexer, '-') == 0) {
        while ((peek(lexer) != '\n') && (lexer->curr < lexer->source_len)) {
          advance(lexer);
        }
      } else {
        add_token(lexer, TokMinus);
      }
      break;
    case '*':
      add_token(lexer, TokStar);
      break;
    case '/':
      add_token(lexer, TokSlash);
      break;
    case '^':
      add_token(lexer, TokCaret);
      break;
    case '%':
      add_token(lexer, TokMod);
      break;
    case ':':
      if (match_char(lexer, '=') == 0) {
        add_token(lexer, TokAssign);
      } else {
        add_token(lexer, TokColon);
      }
      break;
    case ';':
      add_token(lexer, TokSemicolon);
      break;
    case '?':
      add_token(lexer, TokQuestion);
      break;
    case '>':
      if (match_char(lexer, '=') == 0) {
        add_token(lexer, TokGe);
      } else if (match_char(lexer, '>') == 0) {
        add_token(lexer, TokGtgt);
      } else {
        add_token(lexer, TokGt);
      }
      break;
    case '<':
      if (match_char(lexer, '=') == 0) {
        add_token(lexer, TokLe);
      } else if (match_char(lexer, '<') == 0) {
        add_token(lexer, TokLtlt);
      } else {
        add_token(lexer, TokLt);
      }
      break;
    case '=':
      if (match_char(lexer, '=') == 0) {
        add_token(lexer, TokEq);
      }
      break;
    case '~':
      if (match_char(lexer, '=') == 0) {
        add_token(lexer, TokNe);
      } else {
        add_token(lexer, TokNot);
      }
      break;
    case '"':
      handle_string(lexer, ch);
      break;
    case '\'':
      handle_string(lexer, ch);
      break;
    default:
      if isdigit (ch) {
        handle_number(lexer);
      } else if isalnum (ch) {
        handle_identifier(lexer);
      }
      break;
    }
  }
}
