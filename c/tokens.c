#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token token_init(TokenType token_type, char *lexeme, unsigned int line,
                 unsigned int position) {
  return (Token){token_type, lexeme, line, position};
}

char *token_type_string(TokenType token_type) {
  switch (token_type) {
  case (TokLparen):
    return ("TokLparen");
  case (TokRparen):
    return ("TokRparen");
  case (TokLcurly):
    return ("TokLcurly");
  case (TokRcurly):
    return ("TokRcurly");
  case (TokLsquar):
    return ("TokLsquar");
  case (TokRsquar):
    return ("TokRsquar");
  case (TokComma):
    return ("TokComma");
  case (TokDot):
    return ("TokDot");
  case (TokPlus):
    return ("TokPlus");
  case (TokMinus):
    return ("TokMinus");
  case (TokStar):
    return ("TokStar");
  case (TokSlash):
    return ("TokSlash");
  case (TokCaret):
    return ("TokCaret");
  case (TokMod):
    return ("TokMod");
  case (TokColon):
    return ("TokColon");
  case (TokSemicolon):
    return ("TokSemicolon");
  case (TokQuestion):
    return ("TokQuestion");
  case (TokNot):
    return ("TokNot");
  case (TokGt):
    return ("TokGt");
  case (TokLt):
    return ("TokLt");
  case (TokGe):
    return ("TokGe");
  case (TokLe):
    return ("TokLe");
  case (TokNe):
    return ("TokNe");
  case (TokEq):
    return ("TokEq");
  case (TokAssign):
    return ("TokAssign");
  case (TokGtgt):
    return ("TokGtgt");
  case (TokLtlt):
    return ("TokLtlt");
  case (TokIdentifier):
    return ("TokIdentifier");
  case (TokString):
    return ("TokString");
  case (TokInteger):
    return ("TokInteger");
  case (TokFloat):
    return ("TokFloat");
  case (TokIf):
    return ("TokIf");
  case (TokThen):
    return ("TokThen");
  case (TokElse):
    return ("TokElse");
  case (TokTrue):
    return ("TokTrue");
  case (TokFalse):
    return ("TokFalse");
  case (TokAnd):
    return ("TokAnd");
  case (TokOr):
    return ("TokOr");
  case (TokWhile):
    return ("TokWhile");
  case (TokDo):
    return ("TokDo");
  case (TokFor):
    return ("TokFor");
  case (TokFunc):
    return ("TokFunc");
  case (TokNull):
    return ("TokNull");
  case (TokEnd):
    return ("TokEnd");
  case (TokPrint):
    return ("TokPrint");
  case (TokPrintln):
    return ("TokPrintln");
  case (TokRet):
    return ("TokRet");
  }
}

void token_print(Token *token) {
  printf("(%s, %s, on line %d starting at %d)\n",
         token_type_string(token->token_type), token->lexeme, token->line,
         token->position);
}

TokenType keywords(char *lexeme, int lexeme_size) {
  char *checked_lexeme = calloc(sizeof(char), lexeme_size + 1);
  strncpy(checked_lexeme, lexeme, lexeme_size);

  if (strcmp("if", checked_lexeme) == 0) {
    return TokIf;
  } else if (strcmp("else", checked_lexeme) == 0) {
    return TokElse;
  } else if (strcmp("then", checked_lexeme) == 0) {
    return TokThen;
  } else if (strcmp("true", checked_lexeme) == 0) {
    return TokTrue;
  } else if (strcmp("false", checked_lexeme) == 0) {
    return TokFalse;
  } else if (strcmp("and", checked_lexeme) == 0) {
    return TokAnd;
  } else if (strcmp("or", checked_lexeme) == 0) {
    return TokOr;
  } else if (strcmp("while", checked_lexeme) == 0) {
    return TokWhile;
  } else if (strcmp("do", checked_lexeme) == 0) {
    return TokDo;
  } else if (strcmp("for", checked_lexeme) == 0) {
    return TokFor;
  } else if (strcmp("func", checked_lexeme) == 0) {
    return TokFunc;
  } else if (strcmp("null", checked_lexeme) == 0) {
    return TokNull;
  } else if (strcmp("end", checked_lexeme) == 0) {
    return TokEnd;
  } else if (strcmp("print", checked_lexeme) == 0) {
    return TokPrint;
  } else if (strcmp("println", checked_lexeme) == 0) {
    return TokPrintln;
  } else if (strcmp("ret", checked_lexeme) == 0) {
    return TokRet;
  }
  return TokIdentifier;
}
