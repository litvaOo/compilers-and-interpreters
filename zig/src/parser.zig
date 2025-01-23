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

    fn exponent(self: *Parser) *Expression {
        var express = self.factor();
        while (self.match_token(tokens.TokenType.TokCaret)) {
            const token = self.previous_token().?;
            const fact = self.exponent();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = fact } };
            express = new_express;
        }
        return express;
    }

    fn modulo(self: *Parser) *Expression {
        var express = self.exponent();
        while (self.match_token(tokens.TokenType.TokMod)) {
            const token = self.previous_token().?;
            const fact = self.factor();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = fact } };
            express = new_express;
        }
        return express;
    }

    fn term(self: *Parser) *Expression {
        var express = self.modulo();
        while (self.match_token(tokens.TokenType.TokStar) or self.match_token(tokens.TokenType.TokSlash)) {
            const token = self.previous_token().?;
            const fact = self.modulo();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = fact } };
            express = new_express;
        }
        return express;
    }

    fn expr(self: *Parser) *Expression {
        var express = self.term();
        while (self.match_token(tokens.TokenType.TokPlus) or self.match_token(tokens.TokenType.TokMinus)) {
            const token = self.previous_token().?;
            const term_ = self.term();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = term_ } };
            express = new_express;
        }
        return express;
    }

    fn primary(self: *Parser) *Expression {
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
            const express = self.logical_or();
            if (self.match_token(tokens.TokenType.TokRparen)) {
                const result = self.allocator.create(Expression) catch unreachable;
                result.* = Expression{ .Grouping = .{ .value = express } };
                return result;
            }
        }
        const result = self.allocator.create(Expression) catch unreachable;
        result.* = Expression{ .Identifier = .{ .name = self.expect(tokens.TokenType.TokIdentifier).?.lexeme } };
        // std.debug.print("Identifier {s}\n", .{result.*});
        return result;
    }

    fn unary(self: *Parser) *Expression {
        if (self.match_token(tokens.TokenType.TokNot) or self.match_token(tokens.TokenType.TokMinus) or self.match_token(tokens.TokenType.TokPlus)) {
            const token = self.previous_token().?;
            const un = self.unary();
            const result = self.allocator.create(Expression) catch unreachable;
            result.* = Expression{ .UnaryOp = .{ .op = token, .exp = un } };
            return result;
        }
        return self.primary();
    }

    fn factor(self: *Parser) *Expression {
        return self.unary();
    }

    fn comparison(self: *Parser) *Expression {
        var express = self.expr();
        while (self.match_token(tokens.TokenType.TokGe) or self.match_token(tokens.TokenType.TokLe) or self.match_token(tokens.TokenType.TokGt) or self.match_token(tokens.TokenType.TokLt)) {
            const token = self.previous_token().?;
            const and_ = self.expr();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn equality(self: *Parser) *Expression {
        var express = self.comparison();
        while (self.match_token(tokens.TokenType.TokEq) or self.match_token(tokens.TokenType.TokNe)) {
            const token = self.previous_token().?;
            const and_ = self.comparison();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .BinOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn logical_and(self: *Parser) *Expression {
        var express = self.equality();
        while (self.match_token(tokens.TokenType.TokAnd)) {
            const token = self.previous_token().?;
            const and_ = self.logical_and();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .LogicalOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn logical_or(self: *Parser) *Expression {
        var express = self.logical_and();
        while (self.match_token(tokens.TokenType.TokOr)) {
            const token = self.previous_token().?;
            const and_ = self.logical_and();
            const new_express = self.allocator.create(Expression) catch unreachable;
            new_express.* = Expression{ .LogicalOp = .{ .op = token, .left = express, .right = and_ } };
            express = new_express;
        }
        return express;
    }

    fn if_stmt(self: *Parser) !Statement {
        _ = self.expect(tokens.TokenType.TokIf);
        const test_expr = self.logical_or();
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

    fn println_stmt(self: *Parser) Statement {
        if (self.match_token(tokens.TokenType.TokPrintln)) {
            const express = self.logical_or();
            return Statement{ .PrintlnStatement = .{ .value = express.* } };
        }
        unreachable;
    }

    fn print_stmt(self: *Parser) Statement {
        if (self.match_token(tokens.TokenType.TokPrint)) {
            const express = self.logical_or();
            return Statement{ .PrintStatement = .{ .value = express.* } };
        }
        unreachable;
    }

    fn stmt(self: *Parser) !Statement {
        const token = self.peek().?;
        switch (token.token_type) {
            tokens.TokenType.TokPrintln => return self.println_stmt(),
            tokens.TokenType.TokPrint => return self.print_stmt(),
            tokens.TokenType.TokIf => return self.if_stmt(),
            else => {
                // std.debug.print("Going into Assignment\n", .{});
                const left = self.expr();
                // std.debug.print("left is {s}\n", .{left.*});
                if (self.match_token(tokens.TokenType.TokAssign)) {
                    const right = self.expr();
                    // std.debug.print("Setting into Assignment {s} {s}\n", .{ left.*, right.* });
                    return Statement{ .Assignment = .{ .left = left.*, .right = right.* } };
                }
                unreachable;
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
