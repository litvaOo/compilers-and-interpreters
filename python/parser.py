from typing import List
from model import BinOp, Expression, UnaryOp, Float, Grouping, Integer
from tokens import Token, TokenType
from typing import Optional

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.current = 0

    def advance(self) -> Optional[Token]:
        if self.current < len(self.tokens):
            tok = self.tokens[self.current]
            self.current += 1
            return tok
        return None
     
    def peek(self) -> Optional[Token]:
        if self.current < len(self.tokens):
            return self.tokens[self.current]
        return None

    def is_next(self, expected_type: TokenType) -> bool:
        assert isinstance(expected_type, TokenType), expected_type
        tok = self.peek()
        assert tok is not None
        return tok.token_type == expected_type

    def expect(self, expected_type: TokenType) -> Token:
        assert isinstance(expected_type, TokenType), expected_type
        tok = self.peek()
        assert tok is not None
        if tok.token_type == expected_type:
            tok = self.advance()
            assert tok is not None
            return tok
        raise SyntaxError(f'Unexpected token {tok}')

    def match(self, token_type: TokenType) -> bool:
        assert isinstance(token_type, TokenType), token_type
        tok = self.peek()
        if not tok or tok.token_type != token_type:
            return False
        self.current += 1
        return True

    def previous_token(self) -> Optional[Token]:
        if self.current-1 >= 0:
            return self.tokens[self.current-1]
        return None

    def factor(self) -> Expression:
        return self.unary()

    def term(self) -> Expression:
        expr = self.factor()
        while self.match(TokenType.TOK_STAR) or self.match(TokenType.TOK_SLASH):
            op = self.previous_token()
            assert op is not None
            right = self.factor()
            expr = BinOp(op, expr, right)
        return expr

    def unary(self) -> Expression:
        if self.match(TokenType.TOK_NOT) or self.match(TokenType.TOK_MINUS) or self.match(TokenType.TOK_PLUS):
            op = self.previous_token()
            assert op is not None
            operand = self.unary()
            return UnaryOp(op, operand)
        return self.primary()

    def primary(self) -> Expression:
        if self.match(TokenType.TOK_INTEGER):
            tok = self.previous_token()
            assert tok is not None
            return Integer(int(tok.lexeme))
        if self.match(TokenType.TOK_FLOAT):
            tok = self.previous_token()
            assert tok is not None
            return Float(float(tok.lexeme))
        if self.match(TokenType.TOK_LPAREN):
            expr = self.expr()
            if (not self.match(TokenType.TOK_RPAREN)):
                raise SyntaxError(f'Error: ")" expected')
            else:
                return Grouping(expr)
        raise SyntaxError("Unable to parse primary token")

    def expr(self) -> Expression:
        expr = self.term()
        while self.match(TokenType.TOK_PLUS) or self.match(TokenType.TOK_MINUS):
            op = self.previous_token()
            assert op is not None
            right = self.term()
            expr = BinOp(op, expr, right)
        return expr

    def parse(self) -> Expression:
        ast = self.expr()
        return ast 

    def __repr__(self) -> str:
        return str(dir(self))

