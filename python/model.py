from typing import List, Optional
from tokens import Token


class Node:
    def __init__(self) -> None:
        pass


class Expression(Node):
    def __init__(self) -> None:
        self.tabs = 0


class Statement(Node):
    def __init__(self) -> None:
        pass


class Identifier(Expression):
    def __init__(self, name: str) -> None:
        assert isinstance(name, str), name
        self.name = name

    def __repr__(self) -> str:
        return f"Identifier {self.name}"


class Statements(Node):
    def __init__(self, stmts: List[Statement], tabs: int = 0) -> None:
        assert all(isinstance(stmt, Statement) for stmt in stmts), stmts
        self.stmts = stmts
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * self.tabs * 2} Stmts ({self.stmts})"


class PrintStatement(Statement):
    def __init__(self, val: Expression) -> None:
        assert isinstance(val, Expression), val
        self.val = val

    def __repr__(self) -> str:
        return f"PrintStatement({self.val})"


class PrintlnStatement(Statement):
    def __init__(self, val: Expression) -> None:
        assert isinstance(val, Expression), val
        self.val = val

    def __repr__(self) -> str:
        return f"PrintlnStatement({self.val})"


class IfStatement(Statement):
    def __init__(
        self, test: Expression, then_stmts: Statements, else_stmts: Optional[Statements]
    ) -> None:
        assert isinstance(test, Expression), test
        assert isinstance(then_stmts, Statements), then_stmts
        assert else_stmts is None or isinstance(else_stmts, Statements), else_stmts
        self.test = test
        self.then_stmts = then_stmts
        self.else_stmts = else_stmts

    def __repr__(self) -> str:
        return f"IfStmt({self.test}, true: {self.then_stmts}, false: {self.else_stmts})"


class WhileStatement(Statement):
    def __init__(self, test: Expression, stmts: Statements) -> None:
        assert isinstance(test, Expression), test
        assert isinstance(stmts, Statements), stmts
        self.test = test
        self.stmts = stmts

    def __repr__(self) -> str:
        return f"WhileStatement({self.test}, {self.stmts})"


class Assignment(Statement):
    def __init__(self, left: Identifier, right: Expression) -> None:
        assert isinstance(left, Identifier), left
        assert isinstance(right, Expression), right
        self.left = left
        self.right = right

    def __repr__(self) -> str:
        return f"Assignment {self.left} {self.right}"


class LocalAssignment(Statement):
    def __init__(self, left: Identifier, right: Expression) -> None:
        assert isinstance(left, Identifier), left
        assert isinstance(right, Expression), right
        self.left = left
        self.right = right

    def __repr__(self) -> str:
        return f"Assignment {self.left} {self.right}"


class ForStatement(Statement):
    def __init__(
        self, start: Assignment, end: Expression, step: Expression, stmts: Statements
    ) -> None:
        assert isinstance(start, Assignment), start
        assert isinstance(end, Expression), end
        assert isinstance(step, Expression), step
        assert isinstance(stmts, Statements), stmts
        self.start = start
        self.end = end
        self.step = step
        self.stmts = stmts

    def __repr__(self) -> str:
        return f"ForStatement({self.start}, {self.end}, {self.step}, {self.stmts})"


class Bool(Expression):
    def __init__(self, value: bool, tabs: int = 0):
        assert isinstance(value, bool), value
        self.value = value
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * self.tabs * 2}Bool {self.value}"


class String(Expression):
    def __init__(self, value: str, tabs: int = 0):
        assert isinstance(value, str), value
        self.value = value
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * self.tabs * 2}String {self.value}"


class Integer(Expression):
    def __init__(self, value: int, tabs: int = 0) -> None:
        assert isinstance(value, int), value
        self.value = value
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * self.tabs * 2}Integer {self.value}"


class Float(Expression):
    def __init__(self, value: float, tabs: int = 0) -> None:
        assert isinstance(value, float), value
        self.value = value
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * self.tabs * 2}Float {self.value}"


class UnaryOp(Expression):
    def __init__(self, op: Token, exp: Expression, tabs: int = 0) -> None:
        assert isinstance(op, Token), op
        assert isinstance(exp, Expression), exp
        self.op = op
        self.exp = exp
        self.tabs = tabs

    def __repr__(self) -> str:
        self.exp.tabs = self.tabs + 1
        return f"""{" " * (self.tabs * 2)}UnaryOp (
{" " * (self.tabs * 2 + 1)}{self.op.lexeme!r},
{self.exp}
{" " * (self.tabs * 2 + 1)})"""


class BinOp(Expression):
    def __init__(
        self, op: Token, left: Expression, right: Expression, tabs: int = 0
    ) -> None:
        assert isinstance(op, Token), op
        assert isinstance(left, Expression), left
        assert isinstance(right, Expression), right
        self.op = op
        self.left = left
        self.right = right
        self.tabs = tabs

    def __repr__(self) -> str:
        self.right.tabs = self.tabs + 1
        self.left.tabs = self.tabs + 1
        return f"""{" " * (self.tabs * 2)}BinOp (
{" " * (self.tabs * 2 + 1)}{self.op.lexeme!r},
{self.left},
{self.right}
{" " * (self.tabs * 2 + 1)})"""


class LogicalOp(BinOp):
    pass


class Grouping(Expression):
    def __init__(self, value: Expression, tabs: int = 0) -> None:
        assert isinstance(value, Expression), value
        self.value = value
        self.tabs = tabs

    def __repr__(self) -> str:
        self.value.tabs = self.tabs + 1
        return f"""{" " * (self.tabs * 2)}Grouping (
{self.value}
{" " * (self.tabs * 2 + 1)})"""


class Declaration(Statement):
    pass


class Parameter(Declaration):
    def __init__(self, name: str, tabs: int = 0) -> None:
        self.name = name
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * (self.tabs * 2)}Parameter {self.name}"


class FunctionCall(Expression):
    def __init__(self, name: str, args: List[Expression], tabs: int = 0) -> None:
        self.name = name
        self.args = args
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * (self.tabs * 2)}FunctionCall {self.name} with {self.args}"


class FunctionCallStatement(Statement):
    def __init__(self, expr: FunctionCall, tabs: int = 0) -> None:
        assert isinstance(expr, FunctionCall), expr
        self.expr = expr
        self.tabs = tabs

    def __repr__(self) -> str:
        return f"{' ' * (self.tabs * 2)}FunctionCallStatement {self.expr}"


class FunctionDeclaration(Declaration):
    def __init__(
        self, name: str, params: List[Parameter], stmts: Statements, tabs: int = 0
    ) -> None:
        self.name = name
        self.params = params
        self.stmts = stmts
        self.tabs = tabs

    def __repr__(self) -> str:
        self.stmts.tabs += 1
        return f"{' ' * (self.tabs * 2)}FunctionDeclaration {self.name} with params {self.params}\n{self.stmts}"


class ReturnStatement(Statement):
    def __init__(self, val) -> None:
        assert isinstance(val, Expression), val
        self.value = val
