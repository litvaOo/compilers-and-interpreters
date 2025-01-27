from typing import List
from model import (
    Assignment,
    BinOp,
    Identifier,
    IfStatement,
    LogicalOp,
    Bool,
    Expression,
    Statement,
    Statements,
    String,
    UnaryOp,
    Float,
    Grouping,
    Integer,
    PrintStatement,
    PrintlnStatement,
    WhileStatement,
    ForStatement,
)
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
        raise SyntaxError(f"Unexpected token {tok}")

    def match(self, token_type: TokenType) -> bool:
        assert isinstance(token_type, TokenType), token_type
        tok = self.peek()
        if not tok or tok.token_type != token_type:
            return False
        self.current += 1
        return True

    def previous_token(self) -> Optional[Token]:
        if self.current - 1 >= 0:
            return self.tokens[self.current - 1]
        return None

    def factor(self) -> Expression:
        return self.unary()

    def exponent(self) -> Expression:
        expr = self.factor()
        while self.match(TokenType.TOK_CARET):
            op = self.previous_token()
            assert op is not None
            right = self.exponent()

            expr = BinOp(op, expr, right)
        return expr

    def modulo(self) -> Expression:
        expr = self.exponent()
        while self.match(TokenType.TOK_MOD):
            op = self.previous_token()
            assert op is not None
            right = self.factor()
            expr = BinOp(op, expr, right)
        return expr

    def term(self) -> Expression:
        expr = self.modulo()
        while self.match(TokenType.TOK_STAR) or self.match(TokenType.TOK_SLASH):
            op = self.previous_token()
            assert op is not None
            right = self.modulo()
            expr = BinOp(op, expr, right)
        return expr

    def unary(self) -> Expression:
        if (
            self.match(TokenType.TOK_NOT)
            or self.match(TokenType.TOK_MINUS)
            or self.match(TokenType.TOK_PLUS)
        ):
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
        if self.match(TokenType.TOK_TRUE):
            tok = self.previous_token()
            assert tok is not None
            return Bool(True)
        if self.match(TokenType.TOK_FALSE):
            tok = self.previous_token()
            assert tok is not None
            return Bool(False)
        if self.match(TokenType.TOK_STRING):
            tok = self.previous_token()
            assert tok is not None
            return String(tok.lexeme[1:-1])
        if self.match(TokenType.TOK_LPAREN):
            expr = self.logical_or()
            if not self.match(TokenType.TOK_RPAREN):
                raise SyntaxError('Error: ")" expected')
            else:
                return Grouping(expr)
        identifier = self.expect(TokenType.TOK_IDENTIFIER)
        return Identifier(identifier.lexeme)

    def expr(self) -> Expression:
        expr = self.term()
        while self.match(TokenType.TOK_PLUS) or self.match(TokenType.TOK_MINUS):
            op = self.previous_token()
            assert op is not None
            right = self.term()
            expr = BinOp(op, expr, right)
        return expr

    def comparison(self) -> Expression:
        expr = self.expr()
        while (
            self.match(TokenType.TOK_GE)
            or self.match(TokenType.TOK_LE)
            or self.match(TokenType.TOK_GT)
            or self.match(TokenType.TOK_LT)
        ):
            op = self.previous_token()
            assert op is not None
            right = self.expr()
            expr = BinOp(op, expr, right)
        return expr

    def equality(self) -> Expression:
        expr = self.comparison()
        while self.match(TokenType.TOK_EQ) or self.match(TokenType.TOK_NE):
            op = self.previous_token()
            assert op is not None
            right = self.comparison()
            expr = BinOp(op, expr, right)
        return expr

    def logical_and(self) -> Expression:
        expr = self.equality()
        while self.match(TokenType.TOK_AND):
            op = self.previous_token()
            assert op is not None
            right = self.equality()
            expr = LogicalOp(op, expr, right)
        return expr

    def logical_or(self) -> Expression:
        expr = self.logical_and()
        while self.match(TokenType.TOK_OR):
            op = self.previous_token()
            assert op is not None
            right = self.logical_and()
            expr = LogicalOp(op, expr, right)
        return expr

    def print_stmt(self) -> PrintStatement:
        if self.match(TokenType.TOK_PRINT):
            return PrintStatement(self.logical_or())
        assert False, "Wrong token type"

    def println_stmt(self) -> PrintlnStatement:
        if self.match(TokenType.TOK_PRINTLN):
            return PrintlnStatement(self.logical_or())
        assert False, "Wrong token type"

    def if_stmt(self) -> IfStatement:
        self.expect(TokenType.TOK_IF)
        test = self.logical_or()
        self.expect(TokenType.TOK_THEN)
        then_stmts = self.stmts()
        else_stmts = None
        if self.is_next(TokenType.TOK_ELSE):
            self.advance()
            else_stmts = self.stmts()
        self.expect(TokenType.TOK_END)
        return IfStatement(test, then_stmts, else_stmts)

    def while_stmt(self) -> WhileStatement:
        self.expect(TokenType.TOK_WHILE)
        test = self.logical_or()
        self.expect(TokenType.TOK_THEN)
        stmts = self.stmts()
        self.expect(TokenType.TOK_END)
        return WhileStatement(test, stmts)

    def for_stmt(self) -> ForStatement:
        self.expect(TokenType.TOK_FOR)
        start = self.stmt()
        assert isinstance(start, Assignment), start
        self.expect(TokenType.TOK_COMMA)
        end = self.logical_or()
        step: Expression = Integer(1)
        if self.match(TokenType.TOK_COMMA):
            step = self.logical_or()
        self.expect(TokenType.TOK_DO)
        stmts = self.stmts()
        self.expect(TokenType.TOK_END)
        return ForStatement(start, end, step, stmts)

    def stmt(self) -> Statement:
        token = self.peek()
        assert token is not None
        match token.token_type:
            case TokenType.TOK_PRINT:
                return self.print_stmt()
            case TokenType.TOK_PRINTLN:
                return self.println_stmt()
            case TokenType.TOK_IF:
                return self.if_stmt()
            case TokenType.TOK_WHILE:
                return self.while_stmt()
            case TokenType.TOK_FOR:
                return self.for_stmt()
            # case TokenType.TOK_FUNC:
            #     return self.func_stmt()
            case _:
                left = self.expr()
                if self.match(TokenType.TOK_ASSIGN):
                    right = self.expr()
                    assert isinstance(left, Identifier), left
                    return Assignment(left, right)
                else:  # TODO: function call
                    pass
        assert False, f"Should not reach here because {token}"

    def stmts(self) -> Statements:
        stmts = []
        while (
            self.current < len(self.tokens)
            and not self.is_next(TokenType.TOK_END)
            and not self.is_next(TokenType.TOK_ELSE)
        ):
            stmts.append(self.stmt())
        return Statements(stmts)

    def parse(self) -> Statements:
        ast = self.stmts()
        return ast

    def __repr__(self) -> str:
        return str(dir(self))
