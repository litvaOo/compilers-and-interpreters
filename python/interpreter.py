from typing import Any, Tuple
from model import (
    BinOp,
    Bool,
    Float,
    Integer,
    Grouping,
    LogicalOp,
    PrintStatement,
    PrintlnStatement,
    Statements,
    String,
    UnaryOp,
    Node,
)
from tokens import TokenType

TYPE_NUMBER = "TYPE_NUMBER"
TYPE_STRING = "TYPE_STRING"
TYPE_BOOL = "TYPE_BOOL"


class Interpreter:
    def __init__(self):
        pass

    def interpret(self, node: Node) -> Tuple[str, Any]:
        if isinstance(node, Integer) or isinstance(node, Float):
            return (TYPE_NUMBER, float(node.value))

        if isinstance(node, Grouping):
            return self.interpret(node.value)

        if isinstance(node, Bool):
            return TYPE_BOOL, node.value

        if isinstance(node, String):
            return TYPE_STRING, node.value

        if isinstance(node, LogicalOp):
            ltype, lval = self.interpret(node.left)
            if node.op.token_type == TokenType.TOK_OR and lval:
                return (TYPE_BOOL, True)
            if node.op.token_type == TokenType.TOK_AND and not lval:
                return (TYPE_BOOL, False)
            rtype, rval = self.interpret(node.right)
            return (TYPE_BOOL, rval)

        if isinstance(node, BinOp):
            ltype, lval = self.interpret(node.left)
            rtype, rval = self.interpret(node.right)
            if rtype == ltype == TYPE_NUMBER:
                if node.op.token_type == TokenType.TOK_PLUS:
                    return (TYPE_NUMBER, lval + rval)
                elif node.op.token_type == TokenType.TOK_MINUS:
                    return (TYPE_NUMBER, lval - rval)
                elif node.op.token_type == TokenType.TOK_STAR:
                    return (TYPE_NUMBER, lval * rval)
                elif node.op.token_type == TokenType.TOK_SLASH:
                    return (TYPE_NUMBER, lval / rval)
                elif node.op.token_type == TokenType.TOK_MOD:
                    return (TYPE_NUMBER, lval % rval)
                elif node.op.token_type == TokenType.TOK_CARET:
                    return (TYPE_NUMBER, lval**rval)
                elif node.op.token_type == TokenType.TOK_EQ:
                    return (TYPE_BOOL, lval == rval)
                elif node.op.token_type == TokenType.TOK_LE:
                    return (TYPE_BOOL, lval <= rval)
                elif node.op.token_type == TokenType.TOK_LT:
                    return (TYPE_BOOL, lval < rval)
                elif node.op.token_type == TokenType.TOK_GE:
                    return (TYPE_BOOL, lval >= rval)
                elif node.op.token_type == TokenType.TOK_GT:
                    return (TYPE_BOOL, lval > rval)
                elif node.op.token_type == TokenType.TOK_NE:
                    return (TYPE_BOOL, lval != rval)

            if ltype == TYPE_STRING or rtype == TYPE_STRING:
                if node.op.token_type == TokenType.TOK_PLUS:
                    return (TYPE_STRING, str(lval) + str(rval))
                elif node.op.token_type == TokenType.TOK_EQ:
                    if ltype == rtype == TYPE_STRING:
                        return (TYPE_BOOL, lval == rval)

                    return (TYPE_BOOL, False)
                elif node.op.token_type == TokenType.TOK_NE:
                    if ltype == rtype == TYPE_STRING:
                        return (TYPE_BOOL, lval != rval)

                    return (TYPE_BOOL, False)
                elif (
                    node.op.token_type == TokenType.TOK_STAR
                    and rtype == TYPE_NUMBER
                    and int(rval) == rval
                ):
                    return (TYPE_STRING, lval * int(rval))
                assert False, "Unsupported operation"

            if ltype == rtype == TYPE_BOOL:
                if node.op.token_type == TokenType.TOK_EQ:
                    if ltype == rtype == TYPE_BOOL:
                        return (TYPE_BOOL, lval == rval)

                    return (TYPE_BOOL, False)
                elif node.op.token_type == TokenType.TOK_NE:
                    if ltype == rtype == TYPE_BOOL:
                        return (TYPE_BOOL, lval != rval)
                    return (TYPE_BOOL, False)
            if ltype == TYPE_BOOL or rtype == TYPE_BOOL:
                if node.op.token_type == TokenType.TOK_PLUS:
                    return (TYPE_NUMBER, int(lval) + int(rval))
                elif node.op.token_type == TokenType.TOK_MINUS:
                    return (TYPE_NUMBER, int(lval) - int(rval))
                elif node.op.token_type == TokenType.TOK_STAR:
                    return (TYPE_NUMBER, int(lval) * int(rval))
                elif node.op.token_type == TokenType.TOK_CARET:
                    return (TYPE_NUMBER, int(lval) ** int(rval))
            assert False, "Unsupported operation"

        if isinstance(node, UnaryOp):
            rtype, val = self.interpret(node.exp)
            if node.op.token_type == TokenType.TOK_MINUS:
                if rtype == TYPE_NUMBER:
                    return (TYPE_NUMBER, -val)
                assert False, "Unsupported operation"
            if node.op.token_type == TokenType.TOK_PLUS:
                if rtype == TYPE_NUMBER:
                    return (TYPE_NUMBER, +val)
                assert False, "Unsupported operation"
            if node.op.token_type == TokenType.TOK_NOT:
                if rtype == TYPE_BOOL:
                    return (TYPE_BOOL, not val)
                assert False, "Unsupported operation"

        if isinstance(node, Statements):
            for statement in node.stmts:
                self.interpret(statement)
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, PrintStatement):
            express = self.interpret(node.val)
            print(express[1], end="")
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, PrintlnStatement):
            express = self.interpret(node.val)
            print(express[1])
            return (TYPE_NUMBER, 0.0)
        assert False, f"Unknown node type {node}"
