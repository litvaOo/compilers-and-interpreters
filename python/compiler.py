from model import (
    Float,
    Integer,
    Node,
    BinOp,
    PrintlnStatement,
    PrintStatement,
    Statements,
)
from interpreter import TYPE_NUMBER
from tokens import TokenType


class Compiler:
    def __init__(self):
        self.code = []

    def compile(self, node: Node):
        if isinstance(node, Integer) or isinstance(node, Float):
            value = (TYPE_NUMBER, float(node.value))
            self.code.append(("PUSH", value))

        if isinstance(node, BinOp):
            self.compile(node.left)
            self.compile(node.right)
            if node.op.token_type == TokenType.TOK_PLUS:
                self.code.append(("ADD", None))
            if node.op.token_type == TokenType.TOK_MINUS:
                self.code.append(("SUB", None))

        if isinstance(node, Statements):
            for stmt in node.stmts:
                self.compile(stmt)

        if isinstance(node, PrintStatement):
            self.compile(node.val)
            self.code.append(("PRINT", None))
        if isinstance(node, PrintlnStatement):
            self.compile(node.val)
            self.code.append(("PRINTLN", None))

    def compile_code(self, node: Node):
        self.compile(node)
        return self.code
