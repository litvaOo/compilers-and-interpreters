from typing import Any, Tuple
from model import (
    Assignment,
    BinOp,
    Bool,
    Float,
    ForStatement,
    FunctionCall,
    FunctionCallStatement,
    FunctionDeclaration,
    Identifier,
    IfStatement,
    Integer,
    Grouping,
    LogicalOp,
    PrintStatement,
    PrintlnStatement,
    ReturnStatement,
    Statements,
    String,
    UnaryOp,
    Node,
    WhileStatement,
)
from tokens import TokenType
from state import Environment
import codecs

TYPE_NUMBER = "TYPE_NUMBER"
TYPE_STRING = "TYPE_STRING"
TYPE_BOOL = "TYPE_BOOL"
TYPE_RETURN = "TYPE_RETURN"


class Interpreter:
    def __init__(self):
        pass

    def interpret(self, node: Node, env: Environment) -> Tuple[str, Any]:
        if isinstance(node, Integer) or isinstance(node, Float):
            return (TYPE_NUMBER, float(node.value))

        if isinstance(node, Grouping):
            return self.interpret(node.value, env)

        if isinstance(node, Bool):
            return TYPE_BOOL, node.value

        if isinstance(node, String):
            return TYPE_STRING, node.value

        if isinstance(node, Identifier):
            try:
                return env[node.name]
            except KeyError:
                raise SyntaxError(f"Unidentified variable {node.name}")

        if isinstance(node, Assignment):
            rtype, rval = self.interpret(node.right, env)
            env[node.left.name] = (rtype, rval)  # type: ignore
            return (TYPE_NUMBER, 0)

        if isinstance(node, LogicalOp):
            ltype, lval = self.interpret(node.left, env)
            if node.op.token_type == TokenType.TOK_OR and lval:
                return (TYPE_BOOL, True)
            if node.op.token_type == TokenType.TOK_AND and not lval:
                return (TYPE_BOOL, False)
            rtype, rval = self.interpret(node.right, env)
            return (TYPE_BOOL, rval)

        if isinstance(node, BinOp):
            ltype, lval = self.interpret(node.left, env)
            rtype, rval = self.interpret(node.right, env)
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
                    if rtype == TYPE_NUMBER:
                        rval = str(rval if int(rval) != rval else int(rval))
                    if ltype == TYPE_NUMBER:
                        lval = str(lval if int(lval) != lval else int(lval))
                    return (
                        TYPE_STRING,
                        str(lval) + str(rval),
                    )
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
            rtype, val = self.interpret(node.exp, env)
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
                res = self.interpret(statement, env)
                if res[0] == TYPE_RETURN:
                    return res
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, PrintStatement):
            express = self.interpret(node.val, env)
            decoded_buf = codecs.escape_decode(bytes(str(express[1]), "utf-8"))[0]
            print(
                decoded_buf.decode("utf-8"),  # type: ignore # because it actually is bytes, not str as in escape_decode signature
                end="",
            )
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, PrintlnStatement):
            express = self.interpret(node.val, env)
            decoded_buf = codecs.escape_decode(bytes(str(express[1]), "utf-8"))[0]
            print(
                decoded_buf.decode("utf-8"),  # type: ignore # because it actually is bytes, not str as in escape_decode signature
            )
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, IfStatement):
            _, expr_val = self.interpret(node.test, env)
            res = (TYPE_NUMBER, 0.0)
            if expr_val:
                res = self.interpret(node.then_stmts, env.new_env())
            elif node.else_stmts is not None:
                res = self.interpret(node.else_stmts, env.new_env())
            return res

        if isinstance(node, WhileStatement):
            new_state = env.new_env()
            while self.interpret(node.test, new_state)[1]:
                res = self.interpret(node.stmts, new_state)
                if res[0] == TYPE_RETURN:
                    return res
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, ForStatement):
            new_state = env.new_env()
            self.interpret(node.start, new_state)
            end = int(self.interpret(node.end, new_state)[1])
            step = int(self.interpret(node.step, new_state)[1])
            for i in range(int(new_state[node.start.left.name][1]), end, step):
                new_state[node.start.left.name] = (TYPE_NUMBER, i)
                res = self.interpret(node.stmts, new_state)
                if res[0] == TYPE_RETURN:
                    return res
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, FunctionCall):
            func_decl, func_env = env.get_func(node.name)
            assert func_decl is not None, node.name

            assert len(node.args) == len(func_decl.params), node.args

            args = []
            for arg in node.args:
                args.append(self.interpret(arg, env))
            new_state = func_env.new_env()

            for param, arg_val in zip(func_decl.params, args):
                new_state[param.name] = arg_val

            res = self.interpret(func_decl.stmts, new_state)
            if res[0] == TYPE_RETURN:
                res = res[1]
            return res

        if isinstance(node, FunctionCallStatement):
            return self.interpret(node.expr, env)

        if isinstance(node, FunctionDeclaration):
            env.set_func(node.name, (node, env))
            return (TYPE_NUMBER, 0.0)

        if isinstance(node, ReturnStatement):
            return (TYPE_RETURN, self.interpret(node.value, env))
        assert False, f"Unknown node type {node}"

    def interpret_ast(self, node: Node):
        env = Environment()
        self.interpret(node, env)
