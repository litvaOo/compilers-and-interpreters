from collections import defaultdict
from enum import Enum


class TokenType(Enum):
    TOK_LPAREN = "TOK_LPAREN"  #  (
    TOK_RPAREN = "TOK_RPAREN"  #  )
    TOK_LCURLY = "TOK_LCURLY"  #  {
    TOK_RCURLY = "TOK_RCURLY"  #  }
    TOK_LSQUAR = "TOK_LSQUAR"  #  [
    TOK_RSQUAR = "TOK_RSQUAR"  #  ]
    TOK_COMMA = "TOK_COMMA"  #  ,
    TOK_DOT = "TOK_DOT"  #  .
    TOK_PLUS = "TOK_PLUS"  #  +
    TOK_MINUS = "TOK_MINUS"  #  -
    TOK_STAR = "TOK_STAR"  #  *
    TOK_SLASH = "TOK_SLASH"  #  /
    TOK_CARET = "TOK_CARET"  #  ^
    TOK_MOD = "TOK_MOD"  #  %
    TOK_COLON = "TOK_COLON"  #  :
    TOK_SEMICOLON = "TOK_SEMICOLON"  #  ;
    TOK_QUESTION = "TOK_QUESTION"  #  ?
    TOK_NOT = "TOK_NOT"  #  ~
    TOK_GT = "TOK_GT"  #  >
    TOK_LT = "TOK_LT"  #  <
    # Two-char tokens
    TOK_GE = "TOK_GE"  #  >=
    TOK_LE = "TOK_LE"  #  <=
    TOK_NE = "TOK_NE"  #  ~=
    TOK_EQ = "TOK_EQ"  #  ==
    TOK_ASSIGN = "TOK_ASSIGN"  #  :=
    TOK_GTGT = "TOK_GTGT"  #  >>
    TOK_LTLT = "TOK_LTLT"  #  <<
    # Literals
    TOK_IDENTIFIER = "TOK_IDENTIFIER"
    TOK_STRING = "TOK_STRING"
    TOK_INTEGER = "TOK_INTEGER"
    TOK_FLOAT = "TOK_FLOAT"
    # Keywords
    TOK_IF = "TOK_IF"
    TOK_THEN = "TOK_THEN"
    TOK_ELSE = "TOK_ELSE"
    TOK_TRUE = "TOK_TRUE"
    TOK_FALSE = "TOK_FALSE"
    TOK_AND = "TOK_AND"
    TOK_OR = "TOK_OR"
    TOK_WHILE = "TOK_WHILE"
    TOK_DO = "TOK_DO"
    TOK_FOR = "TOK_FOR"
    TOK_FUNC = "TOK_FUNC"
    TOK_NULL = "TOK_NULL"
    TOK_END = "TOK_END"
    TOK_PRINT = "TOK_PRINT"
    TOK_PRINTLN = "TOK_PRINTLN"
    TOK_RET = "TOK_RET"

KEYWORDS = defaultdict(lambda: TokenType.TOK_IDENTIFIER, {
  'if'      : TokenType.TOK_IF,
  'else'    : TokenType.TOK_ELSE,
  'then'    : TokenType.TOK_THEN,
  'true'    : TokenType.TOK_TRUE,
  'false'   : TokenType.TOK_FALSE,
  'and'     : TokenType.TOK_AND,
  'or'      : TokenType.TOK_OR,
  'while'   : TokenType.TOK_WHILE,
  'do'      : TokenType.TOK_DO,
  'for'     : TokenType.TOK_FOR,
  'func'    : TokenType.TOK_FUNC,
  'null'    : TokenType.TOK_NULL,
  'end'     : TokenType.TOK_END,
  'print'   : TokenType.TOK_PRINT,
  'println' : TokenType.TOK_PRINTLN,
  'ret'     : TokenType.TOK_RET,
})

class Token:
    def __init__(self, token_type: TokenType, lexeme: str, line: int, line_position: int) -> None:
        self.token_type = token_type
        self.lexeme = lexeme
        self.line = line
        self.line_position = line_position

    def __repr__(self) -> str:
        return f"({self.token_type}, {self.lexeme!r}, on line {self.line} at position {self.line_position})"
