const std = @import("std");
const tokens = @import("tokens.zig");
const ArrayList = std.ArrayList;
const Expression = @import("model.zig").Expression;
const Node = @import("model.zig").Node;
const Statement = @import("model.zig").Statement;
const Statements = @import("model.zig").Statements;

pub const Parser = struct {
    tokens_list: []tokens.Token,
    current: u32,
    allocator: std.mem.Allocator,

    fn advance(self: *Parser) tokens.Token {
        if (self.current < self.tokens_list.len) {
            self.current += 1;
        }
        return self.tokens_list[self.current - 1];
    }

    fn is_next(self: *Parser, expected_type: tokens.TokenType) bool {
        const token = self.peek();
        if (token == null) return false;
        return token.?.token_type == expected_type;
    }

    fn expect(self: *Parser, expected_type: tokens.TokenType) ?tokens.Token {
        const token = self.peek();
        if (token == null) return null;
        if (token.?.token_type == expected_type) return self.advance();
        unreachable;
    }

    fn peek(self: *Parser) ?tokens.Token {
        if (self.current < self.tokens_list.len) {
            return self.tokens_list[self.current];
        }
        return null;
    }

    fn match_token(self: *Parser, token_type: tokens.TokenType) bool {
        const token = self.peek() orelse return false;
        if (token.token_type == token_type) {
            self.current += 1;
            return true;
        }
        return false;
    }

    fn previous_token(self: *Parser) ?tokens.Token {
        if (self.current >= 1) return self.tokens_list[self.current - 1];
        return null;
    }

    fn exponent(self: *Parser) !*Expression {
        var express = try self.factor();
        while (self.match_token(tokens.TokenType.TokCaret)) {
            const token = self.previous_token().?;
            const fact = try self.exponent();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = fact } };
            express = new_express;
        }
        return express;
    }

    fn modulo(self: *Parser) !*Expression {
        var express = try self.exponent();
        while (self.match_token(tokens.TokenType.TokMod)) {
            const token = self.previous_token().?;
            const fact = try self.factor();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = fact } };
            express = new_express;
        }
        return express;
    }

    fn term(self: *Parser) !*Expression {
        var express = try self.modulo();
        while (self.match_token(tokens.TokenType.TokStar) or self.match_token(tokens.TokenType.TokSlash)) {
            const token = self.previous_token().?;
            const fact = try self.modulo();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = fact } };
            express = new_express;
        }
        return express;
    }

    fn expr(self: *Parser) std.mem.Allocator.Error!*Expression {
        var express = try self.term();
        while (self.match_token(tokens.TokenType.TokPlus) or self.match_token(tokens.TokenType.TokMinus)) {
            const token = self.previous_token().?;
            const term_ = try self.term();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = term_ } };
            express = new_express;
        }
        return express;
    }

    fn primary(self: *Parser) !*Expression {
        if (self.match_token(tokens.TokenType.TokInteger)) {
            const token = self.previous_token().?;
            const lexeme_value = std.fmt.parseInt(i32, token.lexeme, 10) catch unreachable;
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .Integer = .{ .value = lexeme_value } };
            return result;
        }
        if (self.match_token(tokens.TokenType.TokFloat)) {
            const token = self.previous_token().?;
            const lexeme_value = std.fmt.parseFloat(f64, token.lexeme) catch unreachable;
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .Float = .{ .value = lexeme_value } };
            return result;
        }
        if (self.match_token(tokens.TokenType.TokTrue)) {
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .Bool = .{ .value = true } };
            return result;
        }
        if (self.match_token(tokens.TokenType.TokFalse)) {
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .Bool = .{ .value = false } };
            return result;
        }
        if (self.match_token(tokens.TokenType.TokString)) {
            const token = self.previous_token().?;
            const len = token.lexeme.len;
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .String = .{ .value = token.lexeme[1 .. len - 1] } };
            return result;
        }
        if (self.match_token(tokens.TokenType.TokLparen)) {
            const express = try self.logical_or();
            if (self.match_token(tokens.TokenType.TokRparen)) {
                const result = self.allocator.create(Expression) catch unreachable;
                result.* = Expression{ .Grouping = .{ .value = express } };
                return result;
            }
        }
        const result = self.allocator.create(Expression) catch unreachable;
        const identifier = self.expect(tokens.TokenType.TokIdentifier).?.lexeme;
        if (self.match_token(tokens.TokenType.TokLparen)) {
            const args = try self.function_params();
            _ = self.expect(tokens.TokenType.TokRparen);
            result.* = Expression{ .FunctionCall = .{ .name = identifier, .args = args } };
        } else {
            result.* = Expression{ .Identifier = .{ .name = identifier } };
        }
        return result;
    }

    fn function_params(self: *Parser) !ArrayList(Expression) {
        var args = ArrayList(Expression).init(self.allocator);
        while (!self.is_next(tokens.TokenType.TokRparen)) {
            try args.append((try self.logical_or()).*);
            if (!self.is_next(tokens.TokenType.TokRparen)) {
                _ = self.expect(tokens.TokenType.TokComma);
            }
        }
        return args;
    }

    fn unary(self: *Parser) !*Expression {
        if (self.match_token(tokens.TokenType.TokNot) or self.match_token(tokens.TokenType.TokMinus) or self.match_token(tokens.TokenType.TokPlus)) {
            const token = self.previous_token().?;
            const un = try self.unary();
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .UnaryOp = .{ .op = token, .exp = un } };
            return result;
        }
        return try self.primary();
    }

    fn factor(self: *Parser) !*Expression {
        return try self.unary();
    }

    fn comparison(self: *Parser) !*Expression {
        var express = try self.expr();
        while (self.match_token(tokens.TokenType.TokGe) or self.match_token(tokens.TokenType.TokLe) or self.match_token(tokens.TokenType.TokGt) or self.match_token(tokens.TokenType.TokLt)) {
            const token = self.previous_token().?;
            const and_ = try self.expr();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn equality(self: *Parser) !*Expression {
        var express = try self.comparison();
        while (self.match_token(tokens.TokenType.TokEq) or self.match_token(tokens.TokenType.TokNe)) {
            const token = self.previous_token().?;
            const and_ = try self.comparison();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn logical_and(self: *Parser) !*Expression {
        var express = try self.equality();
        while (self.match_token(tokens.TokenType.TokAnd)) {
            const token = self.previous_token().?;
            const and_ = try self.logical_and();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .LogicalOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn logical_or(self: *Parser) !*Expression {
        var express = try self.logical_and();
        while (self.match_token(tokens.TokenType.TokOr)) {
            const token = self.previous_token().?;
            const and_ = try self.logical_and();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .LogicalOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn if_stmt(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokIf);
        const test_expr = try self.logical_or();
        _ = self.expect(tokens.TokenType.TokThen);
        const then_stmts = try self.stmts();
        var else_stmts = Statements.init(self.allocator);
        if (self.is_next(tokens.TokenType.TokElse)) {
            _ = self.advance();
            else_stmts = try self.stmts();
        }
        _ = self.expect(tokens.TokenType.TokEnd);
        return Statement{ .IfStatement = .{ .test_expr = test_expr.*, .then_stmts = then_stmts, .else_stmts = else_stmts } };
    }

    fn while_stmt(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokWhile);
        const test_expr = try self.logical_or();
        _ = self.expect(tokens.TokenType.TokThen);
        const while_stmts = try self.stmts();
        _ = self.expect(tokens.TokenType.TokEnd);
        return Statement{ .While = .{ .test_expr = test_expr.*, .stmts = while_stmts } };
    }

    fn for_stmt(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokFor);
        const id = try self.primary();
        _ = self.expect(tokens.TokenType.TokAssign);
        const start = try self.logical_or();
        _ = self.expect(tokens.TokenType.TokComma);
        const end = try self.logical_or();
        var step = &Expression{ .Integer = .{ .value = 1 } };
        if (self.match_token(tokens.TokenType.TokComma)) {
            step = try self.logical_or();
        }
        _ = self.expect(tokens.TokenType.TokDo);
        const for_stmts = try self.stmts();
        _ = self.expect(tokens.TokenType.TokEnd);
        return Statement{ .For = .{ .id = id.*, .start = start.*, .end = end.*, .step = step.*, .stmts = for_stmts } };
    }

    fn println_stmt(self: *Parser) !Statement {
        if (self.match_token(tokens.TokenType.TokPrintln)) {
            const express = try self.logical_or();
            return Statement{ .PrintlnStatement = .{ .value = express.* } };
        }
        unreachable;
    }

    fn print_stmt(self: *Parser) !Statement {
        if (self.match_token(tokens.TokenType.TokPrint)) {
            const express = try self.logical_or();
            return Statement{ .PrintStatement = .{ .value = express.* } };
        }
        unreachable;
    }

    fn ret(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokRet);
        const new_expr = try self.expr();
        return Statement{ .Return = .{ .val = new_expr.* } };
    }

    fn params(self: *Parser) !ArrayList(Statement) {
        var args = ArrayList(Statement).init(self.allocator);
        while (!self.is_next(tokens.TokenType.TokRparen)) {
            const name = self.expect(tokens.TokenType.TokIdentifier).?.lexeme;
            try args.append(Statement{ .Parameter = .{ .name = name } });
            if (!self.is_next(tokens.TokenType.TokRparen)) {
                _ = self.expect(tokens.TokenType.TokComma);
            }
        }
        return args;
    }

    fn function_declaration(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokFunc);
        const name = self.expect(tokens.TokenType.TokIdentifier).?.lexeme;
        _ = self.expect(tokens.TokenType.TokLparen);
        const args = try self.params();
        _ = self.expect(tokens.TokenType.TokRparen);
        const func_stmts = try self.stmts();
        _ = self.expect(tokens.TokenType.TokEnd);
        return Statement{ .FunctionDeclaration = .{
            .name = name,
            .params = args,
            .stmts = func_stmts,
        } };
    }

    fn local_assignment(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokLocal);
        const left = try self.expr();
        _ = self.match_token(tokens.TokenType.TokAssign);
        const right = try self.expr();
        return Statement{ .LocalAssignment = .{ .left = left.*, .right = right.* } };
    }

    fn stmt(self: *Parser) !Statement {
        const token = self.peek().?;
        switch (token.token_type) {
            tokens.TokenType.TokPrintln => return self.println_stmt(),
            tokens.TokenType.TokPrint => return self.print_stmt(),
            tokens.TokenType.TokWhile => return self.while_stmt(),
            tokens.TokenType.TokFor => return self.for_stmt(),
            tokens.TokenType.TokIf => return self.if_stmt(),
            tokens.TokenType.TokRet => return self.ret(),
            tokens.TokenType.TokFunc => return self.function_declaration(),
            tokens.TokenType.TokLocal => return self.local_assignment(),
            else => {
                const left = try self.expr();
                if (self.match_token(tokens.TokenType.TokAssign)) {
                    const right = try self.expr();
                    return Statement{ .Assignment = .{ .left = left.*, .right = right.* } };
                }
                return Statement{ .FunctionCall = .{ .expr = left.* } };
            },
        }
    }

    fn stmts(self: *Parser) std.mem.Allocator.Error!Statements {
        var stmts_arr = Statements.init(self.allocator);
        while (self.current < self.tokens_list.len and !self.is_next(tokens.TokenType.TokEnd) and !self.is_next(tokens.TokenType.TokElse)) {
            try stmts_arr.append(try self.stmt());
        }
        return stmts_arr;
    }

    pub fn parse(self: *Parser) !Node {
        return Node{ .Stmts = try self.stmts() };
    }
};
