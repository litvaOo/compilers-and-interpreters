#include "tokens.h"

Token token_init(char *token_type, char *lexeme) {
  return (Token){token_type, lexeme};
}
