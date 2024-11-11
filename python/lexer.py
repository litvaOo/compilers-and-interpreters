from tokens import Token, TokenType, KEYWORDS
from typing import List


class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.start = 0
        self.curr = 0
        self.line = 1
        self.tokens: List[Token] = []

    def advance(self) -> str:
        if self.curr >= len(self.source):
            return '\0'
        ch = self.source[self.curr]
        self.curr = self.curr + 1
        return ch

    def add_token(self, token_type: TokenType):
        self.tokens.append(
            Token(token_type, self.source[self.start : self.curr], self.line)
        )

    def peek(self) -> str:
        if self.curr >= len(self.source):
            return '\0'
        return self.source[self.curr]

    def lookahead(self, n=1) -> str:
        if self.curr+n >= len(self.source):
            return "\0"
        return self.source[self.curr + n]

    def match(self, expected) -> bool:
        if self.curr >= len(self.source) or self.source[self.curr] != expected:
            return False
        self.curr += 1
        return True

    def handle_number(self):
        while self.peek().isdigit():
            self.advance()
        if self.peek() == '.' and self.lookahead().isdigit():
            self.advance()
            while self.peek().isdigit():
                self.advance()
            self.add_token(TokenType.TOK_FLOAT)
        else:
            self.add_token(TokenType.TOK_INTEGER)

    def handle_identifier(self):
        while self.peek().isalnum() or self.peek() == '_':
            self.advance()
        self.add_token(KEYWORDS[self.source[self.start:self.curr]])

    def handle_string(self, ch: str):
        while self.peek() != ch and self.curr < len(self.source):
            self.advance()
        if self.curr >= len(self.source):
            raise SystemError(f"Unclosed quote on line {self.line}")
        self.advance()
        self.add_token(TokenType.TOK_STRING)


    def tokenize(self):
        while self.curr < len(self.source):
            self.start = self.curr
            ch = self.advance()
            match ch:
                case "\n":
                    self.line += 1
                case "#":
                    while self.peek() != "\n" and not(self.curr >= len(self.source)):
                        self.advance()
                case "(":
                    self.add_token(TokenType.TOK_LPAREN)
                case ")":
                    self.add_token(TokenType.TOK_RPAREN)
                case "{":
                    self.add_token(TokenType.TOK_LCURLY)
                case "}":
                    self.add_token(TokenType.TOK_RCURLY)
                case "[":
                    self.add_token(TokenType.TOK_LSQUAR)
                case "]":
                    self.add_token(TokenType.TOK_RSQUAR)
                case ",":
                    self.add_token(TokenType.TOK_COMMA)
                case ".":
                    self.add_token(TokenType.TOK_DOT)
                case "+":
                    self.add_token(TokenType.TOK_PLUS)
                case "-":
                    if self.match("-"):
                        while self.peek() != "\n" and self.curr < len(self.source):
                            self.advance()
                    else:
                        self.add_token(TokenType.TOK_MINUS)
                case "*":
                    self.add_token(TokenType.TOK_STAR)
                case "/":
                    self.add_token(TokenType.TOK_SLASH)
                case "^":
                    self.add_token(TokenType.TOK_CARET)
                case "%":
                    self.add_token(TokenType.TOK_MOD)
                case ":":
                    if self.match("="):
                        self.add_token(TokenType.TOK_ASSIGN)
                    else:
                        self.add_token(TokenType.TOK_COLON)
                case ";":
                    self.add_token(TokenType.TOK_SEMICOLON)
                case "?":
                    self.add_token(TokenType.TOK_QUESTION)
                case ">":
                    if self.match("="):
                        self.add_token(TokenType.TOK_GE)
                    elif self.match(">"):
                        self.add_token(TokenType.TOK_GTGT)
                    else:
                        self.add_token(TokenType.TOK_GT)
                case "<":
                    if self.match("="):
                        self.add_token(TokenType.TOK_LE)
                    elif self.match("<"):
                        self.add_token(TokenType.TOK_LTLT)
                    else:
                        self.add_token(TokenType.TOK_LT)
                case "=":
                    if self.match("="):
                        self.add_token(TokenType.TOK_EQ)
                case "~":
                    if self.match("="):
                        self.add_token(TokenType.TOK_NE)
                    else:
                        self.add_token(TokenType.TOK_NOT)
                case '"':
                    self.handle_string(ch)
                case "'":
                    self.handle_string(ch)
                case _:
                    if ch.isdigit():
                        self.handle_number()
                    if ch.isalpha():
                        self.handle_identifier()
