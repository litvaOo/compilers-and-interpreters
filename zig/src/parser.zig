const std = @import("std");
const tokens = @import("tokens.zig");
const ArrayList = std.ArrayList;
const Expression = @import("model.zig").Expression;

pub const Parser = struct {
    tokens_list: []tokens.Token,
    current: u32,
    allocator: std.mem.Allocator,

    fn advance(self: *Parser) tokens.Token {
        if (self.current < self.tokens_list.len) {
            self.current += 1;
        }
        return self.tokens_list[self.current];
    }

    fn is_next(self: *Parser, expected_type: tokens.TokenType) bool {
        const token = self.peek();
        if (token == null) return false;
        return token.token_type == expected_type;
    }

    fn expect(self: *Parser, expected_type: tokens.TokenType) ?tokens.Token {
        const token = self.peek();
        if (token == null) return null;
        if (token.token_type == expected_type) return self.advance();
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

    fn term(self: *Parser) *Expression {
        var express = self.factor();
        while (self.match_token(tokens.TokenType.TokStar) or self.match_token(tokens.TokenType.TokSlash)) {
            const token = self.previous_token().?;
            const fact = self.factor();
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
        if (self.match_token(tokens.TokenType.TokLparen)) {
            const express = self.term();
            if (self.match_token(tokens.TokenType.TokRparen)) {
                const result = self.allocator.create(Expression) catch unreachable;
                result.* = Expression{ .Grouping = .{ .value = express } };
                return result;
            }
        }
        unreachable;
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

    pub fn parse(self: *Parser) Expression {
        const result = self.expr();
        // std.debug.print("{}", .{result});
        return result.*;
    }
};
