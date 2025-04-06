from typing import Any, Optional, Tuple, List
from model import (
    Assignment,
    Float,
    FunctionCall,
    FunctionCallStatement,
    FunctionDeclaration,
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
    WhileStatement,
)
from interpreter import TYPE_NUMBER, TYPE_STRING
from tokens import TokenType


SYM_VAR = "SYM_VAR"
SYM_FUNC = "SYM_FUNC"


class Symbol:
    def __init__(self, name: str, depth: int = 0, symbol_type: str = SYM_VAR) -> None:
        self.name = name
        self.depth = depth
        self.symbol_type = symbol_type


class Compiler:
    def __init__(self) -> None:
        self.code: List[Tuple[str, Any]] = []
        self.globals = []
        self.locals = []
        self.functions = []
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
                new_symbol = Symbol(node.left.name, self.scope_depth, SYM_VAR)
                if self.scope_depth == 0:
                    self.globals.append(new_symbol)
                    self.code.append(("STORE_GLOBAL", (None, self.num_globals)))
                    self.num_globals += 1
                else:
                    self.locals.append(new_symbol)
                    self.code.append(("STORE_LOCAL", (None, self.num_locals)))
                    self.num_locals += 1
            else:
                symbol, slot = res
                if symbol.depth == 0:
                    self.code.append(("STORE_GLOBAL", (None, slot)))
                else:
                    self.code.append(("STORE_LOCAL", (None, slot)))

        elif isinstance(node, Identifier):
            res = self.get_symbol(node.name)
            assert res is not None, "Should have this symbol already"
            symbol, slot = res
            if symbol.depth == 0:
                self.code.append(("LOAD_GLOBAL", (None, slot)))
            else:
                self.code.append(("LOAD_LOCAL", (None, slot)))

        elif isinstance(node, WhileStatement):
            test_label = self.make_label()
            exit_label = self.make_label()
            self.code.append(("LABEL", test_label))
            self.compile(node.test)
            self.code.append(("JMPZ", (None, exit_label)))
            self.scope_depth += 1
            self.compile(node.stmts)
            self.end_block()
            self.code.append(("JMP", (None, test_label)))
            self.code.append(("LABEL", exit_label))

        elif isinstance(node, FunctionCall):
            self.code.append(("JSR", (None, node.name)))

        elif isinstance(node, FunctionCallStatement):
            self.compile(node.expr)

        elif isinstance(node, FunctionDeclaration):
            assert self.get_func_symbol(node.name) is None
            new_func = Symbol(node.name, self.scope_depth, SYM_FUNC)
            self.functions.append(new_func)

            end_label = self.make_label()
            self.code.append(("JMP", (None, end_label)))
            self.code.append(("LABEL", new_func.name))
            self.scope_depth += 1
            self.compile(node.stmts)
            self.end_block()
            self.code.append(("RTS", None))
            self.code.append(("LABEL", end_label))

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
                self.locals.remove(i)
            else:
                break

    def get_func_symbol(self, name: str) -> Optional[Symbol]:
        for symbol in self.functions:
            if symbol.name == name:
                return symbol
        return None

    def get_symbol(self, name: str) -> Optional[Tuple[Symbol, int]]:
        for index, symbol in enumerate(self.locals[::-1]):
            if symbol.name == name:
                return (symbol, index)
        for index, symbol in enumerate(self.globals[::-1]):
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
