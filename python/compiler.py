from typing import Any, Optional, Tuple, List
from model import (
    Assignment,
    Float,
    Grouping,
    Identifier,
    IfStatement,
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
from interpreter import TYPE_NUMBER, TYPE_STRING
from tokens import TokenType


class Symbol:
    def __init__(self, name: str, depth: int = 0) -> None:
        self.name = name
        self.depth = depth


class Compiler:
    def __init__(self) -> None:
        self.code: List[Tuple[str, Any]] = []
        self.globals = []
        self.locals = []
        self.num_locals = 0
        self.num_globals = 0
        self.label_counter = 0
        self.scope_depth = 0

    def make_label(self) -> str:
        self.label_counter += 1
        return f"LBL{self.label_counter}"

    def compile(self, node: Node):
        if (
            isinstance(node, Integer)
            or isinstance(node, Float)
            or isinstance(node, Bool)
        ):
            self.code.append(("PUSH", (TYPE_NUMBER, float(node.value))))

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

        elif isinstance(node, IfStatement):
            self.compile(node.test)

            then_label = self.make_label()
            else_label = self.make_label()
            exit_label = self.make_label()
            self.code.append(("JMPZ", (None, else_label)))
            self.code.append(("LABEL", then_label))

            self.scope_depth += 1
            self.compile(node.then_stmts)
            self.end_block()

            self.code.append(("JMP", (None, exit_label)))
            self.code.append(("LABEL", else_label))

            if node.else_stmts:
                self.scope_depth += 1
                self.compile(node.else_stmts)
                self.end_block()

            self.code.append(("LABEL", exit_label))

        elif isinstance(node, Assignment):
            self.compile(node.right)
            res = self.get_symbol(node.left.name)
            if not res:
                new_symbol = Symbol(node.left.name, self.scope_depth)
                if self.scope_depth == 0:
                    self.globals.append(new_symbol)
                    self.code.append(("STORE_GLOBAL", (None, new_symbol.name)))
                    self.num_globals += 1
                else:
                    self.locals.append(new_symbol)
                    self.code.append(("STORE_LOCAL", (None, self.num_locals)))
                    self.num_locals += 1
            else:
                symbol, slot = res
                if symbol.depth == 0:
                    self.code.append(("STORE_GLOBAL", (None, res[0].name)))
                else:
                    self.code.append(("STORE_LOCAL", (None, slot)))

        elif isinstance(node, Identifier):
            res = self.get_symbol(node.name)
            assert res is not None, "Should have this symbol already"
            symbol, slot = res
            if symbol.depth == 0:
                self.code.append(("LOAD_GLOBAL", (None, symbol.name)))
            else:
                self.code.append(("LOAD_LOCAL", (None, slot)))

    def compile_code(self, node):
        self.code.append(("LABEL", "START"))
        self.compile(node)
        self.code.append(("HALT", None))
        return self.code

    def end_block(self):
        self.scope_depth -= 1
        for i in self.locals[::-1]:
            if i.depth > self.scope_depth:
                self.code.append(("POP", None))
                self.num_locals -= 1
                del i
            else:
                break

    def get_symbol(self, name: str) -> Optional[Tuple[Symbol, int]]:
        for index, symbol in enumerate(self.locals):
            if symbol.name == name:
                return (symbol, index)
        for index, symbol in enumerate(self.globals):
            if symbol.name == name:
                return (symbol, index)
        return None

    def __str__(self) -> str:
        res = ""
        for instruction in self.code:
            if instruction[0] == "LABEL":
                res += " ".join(instruction) + ":\n"
            else:
                res += "\t" + instruction[0]
                if instruction[1]:
                    res += " " + str(instruction[1][1])
                res += "\n"
        return res
