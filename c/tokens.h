#pragma once

typedef enum {
  TokLparen,
  TokRparen,
  TokLcurly,
  TokRcurly,
  TokLsquar,
  TokRsquar,
  TokComma,
  TokDot,
  TokPlus,
  TokMinus,
  TokStar,
  TokSlash,
  TokCaret,
  TokMod,
  TokColon,
  TokSemicolon,
  TokQuestion,
  TokNot,
  TokGt,
  TokLt,
  TokGe,
  TokLe,
  TokNe,
  TokEq,
  TokAssign,
  TokGtgt,
  TokLtlt,
  TokIdentifier,
  TokString,
  TokInteger,
  TokFloat,
  TokIf,
  TokThen,
  TokElse,
  TokTrue,
  TokFalse,
  TokAnd,
  TokOr,
  TokWhile,
  TokDo,
  TokFor,
  TokFunc,
  TokNull,
  TokEnd,
  TokPrint,
  TokPrintln,
  TokRet,
  TokLocal,
} TokenType;

typedef struct {
  TokenType token_type;
  char *lexeme;
  unsigned int line;
  unsigned int lexeme_len;
  unsigned int position;
} Token __attribute__((aligned(8)));

inline Token token_init(TokenType token_type, char *lexeme, unsigned int line,
                        unsigned int lexeme_len, unsigned int position) {
  return (Token){token_type, lexeme, line, lexeme_len, position};
}

TokenType keywords(char *lexeme, int lexeme_size);

void token_print(Token *token);
