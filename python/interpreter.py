from typing import Any, Tuple
from model import (
    BinOp,
    Bool,
    Expression,
    Float,
    Integer,
    Grouping,
    LogicalOp,
    String,
    UnaryOp,
)
from tokens import TokenType

TYPE_NUMBER = "TYPE_NUMBER"
TYPE_STRING = "TYPE_STRING"
TYPE_BOOL = "TYPE_BOOL"


class Interpreter:
    def __init__(self):
        pass

    def interpret(self, node: Expression) -> Tuple[str, Any]:
        if isinstance(node, Integer) or isinstance(node, Float):
            return (TYPE_NUMBER, float(node.value))

        elif isinstance(node, Grouping):
            return self.interpret(node.value)

        elif isinstance(node, Bool):
            return TYPE_BOOL, node.value

        elif isinstance(node, String):
            return TYPE_STRING, node.value

        elif isinstance(node, LogicalOp):
            ltype, lval = self.interpret(node.left)
            if node.op.token_type == TokenType.TOK_OR and lval:
                return (TYPE_BOOL, True)
            if node.op.token_type == TokenType.TOK_AND and not lval:
                return (TYPE_BOOL, False)
            rtype, rval = self.interpret(node.right)
            return (TYPE_BOOL, rval)

        elif isinstance(node, BinOp):
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

            elif ltype == TYPE_STRING or rtype == TYPE_STRING:
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
                print(ltype, lval, rtype, rval)
                assert False, "Unsupported operation"
            elif ltype == rtype == TYPE_BOOL:
                if node.op.token_type == TokenType.TOK_EQ:
                    if ltype == rtype == TYPE_BOOL:
                        return (TYPE_BOOL, lval == rval)

                    return (TYPE_BOOL, False)
                elif node.op.token_type == TokenType.TOK_NE:
                    if ltype == rtype == TYPE_BOOL:
                        return (TYPE_BOOL, lval != rval)
                    return (TYPE_BOOL, False)
            elif ltype == TYPE_BOOL or rtype == TYPE_BOOL:
                if node.op.token_type == TokenType.TOK_PLUS:
                    return (TYPE_NUMBER, int(lval) + int(rval))
                elif node.op.token_type == TokenType.TOK_MINUS:
                    return (TYPE_NUMBER, int(lval) - int(rval))
                elif node.op.token_type == TokenType.TOK_STAR:
                    return (TYPE_NUMBER, int(lval) * int(rval))
                elif node.op.token_type == TokenType.TOK_CARET:
                    return (TYPE_NUMBER, int(lval) ** int(rval))
            print(ltype, lval, rtype, rval)
            assert False, "Unsupported operation"

        elif isinstance(node, UnaryOp):
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
        print(type(node))
        # print(node)
        assert False, "Unknown node type"
