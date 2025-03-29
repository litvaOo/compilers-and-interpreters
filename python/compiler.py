from typing import Any, Tuple, List
from model import (
    Float,
    Grouping,
    Integer,
    Node,
    BinOp,
    Bool,
    PrintlnStatement,
    PrintStatement,
    Statements,
    String,
    UnaryOp,
    LogicalOp,
)
from interpreter import TYPE_NUMBER, TYPE_BOOL, TYPE_STRING
from tokens import TokenType


class Compiler:
    def __init__(self) -> None:
        self.code: List[Tuple[str, Any]] = []

    def compile(self, node: Node):
        if isinstance(node, Integer) or isinstance(node, Float):
            self.code.append(("PUSH", (TYPE_NUMBER, float(node.value))))

        elif isinstance(node, Bool):
            self.code.append(("PUSH", (TYPE_BOOL, node.value)))

        elif isinstance(node, String):
            self.code.append(("PUSH", (TYPE_STRING, str(node.value))))

        elif isinstance(node, BinOp):
            self.compile(node.left)
            self.compile(node.right)
            if node.op.token_type == TokenType.TOK_PLUS:
                self.code.append(("ADD", None))
            elif node.op.token_type == TokenType.TOK_MINUS:
                self.code.append(("SUB", None))
            elif node.op.token_type == TokenType.TOK_STAR:
                self.code.append(("MUL", None))
            elif node.op.token_type == TokenType.TOK_SLASH:
                self.code.append(("DIV", None))
            elif node.op.token_type == TokenType.TOK_CARET:
                self.code.append(("EXP", None))
            elif node.op.token_type == TokenType.TOK_MOD:
                self.code.append(("MOD", None))
            elif node.op.token_type == TokenType.TOK_LT:
                self.code.append(("LT", None))
            elif node.op.token_type == TokenType.TOK_GT:
                self.code.append(("GT", None))
            elif node.op.token_type == TokenType.TOK_LE:
                self.code.append(("LE", None))
            elif node.op.token_type == TokenType.TOK_GE:
                self.code.append(("GE", None))
            elif node.op.token_type == TokenType.TOK_EQ:
                self.code.append(("EQ", None))
            elif node.op.token_type == TokenType.TOK_NE:
                self.code.append(("NE", None))

        elif isinstance(node, UnaryOp):
            self.compile(node.exp)
            if node.op.token_type == TokenType.TOK_MINUS:
                self.code.append(("NEG", None))
            elif node.op.token_type == TokenType.TOK_NOT:
                self.code.append(("PUSH", (TYPE_NUMBER, 1)))
                self.code.append(("XOR", None))

        elif isinstance(node, LogicalOp):
            self.compile(node.left)
            self.compile(node.right)
            if node.op.token_type == TokenType.TOK_AND:
                self.code.append(("AND", None))
            elif node.op.token_type == TokenType.TOK_OR:
                self.code.append(("OR", None))

        elif isinstance(node, PrintlnStatement):
            self.compile(node.val)
            self.code.append(("PRINTLN", None))

        elif isinstance(node, PrintStatement):
            self.compile(node.val)
            self.code.append(("PRINT", None))

        elif isinstance(node, Statements):
            for stmt in node.stmts:
                self.compile(stmt)

        elif isinstance(node, Grouping):
            self.compile(node.value)

    def compile_code(self, node):
        self.code.append(("LABEL", "START"))
        self.compile(node)
        self.code.append(("HALT", None))
        return self.code

    def __str__(self) -> str:
        tabs = 0
        res = ""
        for instruction in self.code:
            if instruction[0] == "LABEL":
                res += instruction[1] + ":\n"
                tabs += 1
            else:
                res += tabs * "\t" + instruction[0]
                if instruction[1]:
                    res += " " + str(instruction[1][1])
                res += "\n"
        return res
