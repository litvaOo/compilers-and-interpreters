const Expression = @import("model.zig").Expression;
const TokenType = @import("tokens.zig").TokenType;
const strEql = @import("std").mem.eql;
const strCopy = @import("std").mem.copyForwards;
const Allocator = @import("std").mem.Allocator;
const std = @import("std");
const allocPrint = @import("std").fmt.allocPrint;
const pow = @import("std").math.pow;

pub const ResultType = union(enum) {
    Bool: bool,
    Number: f64,
    String: []const u8,

    pub fn format(self: ResultType, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        switch (self) {
            .Bool => |x| try writer.print("{}", .{x}),
            .Number => |x| try writer.print("{d}", .{x}),
            .String => |x| try writer.print("{s}", .{x}),
        }
    }
};

pub const Interpreter = struct {
    pub fn interpret(self: *Interpreter, node: Expression, allocator: Allocator) !ResultType {
        switch (node) {
            .Integer => |x| return ResultType{ .Number = @floatFromInt(x.value) },
            .Float => |x| return ResultType{ .Number = x.value },
            .Bool => |x| return ResultType{ .Bool = x.value },
            .String => |x| return ResultType{ .String = x.value },
            .Grouping => |x| return try self.interpret(x.value.*, allocator),
            .UnaryOp => |x| {
                const rres = try self.interpret(x.exp.*, allocator);
                switch (rres) {
                    .Number => |rval| {
                        if (x.op.token_type == TokenType.TokMinus) return ResultType{ .Number = -rval };
                        if (x.op.token_type == TokenType.TokPlus) return ResultType{ .Number = rval };
                    },
                    .Bool => |rval| {
                        if (x.op.token_type == TokenType.TokNot) return ResultType{ .Bool = !rval };
                    },
                    else => unreachable,
                }
            },
            .LogicalOp => |x| {
                const lres = try self.interpret(x.left.*, allocator);
                switch (lres) {
                    .Bool => |lval| {
                        if (x.op.token_type == TokenType.TokOr and lval == true) return ResultType{ .Bool = true };
                        if (x.op.token_type == TokenType.TokAnd and lval == false) return ResultType{ .Bool = false };
                        return try self.interpret(x.right.*, allocator);
                    },
                    else => unreachable,
                }
            },
            .BinOp => |x| {
                const lres = try self.interpret(x.left.*, allocator);
                const rres = try self.interpret(x.right.*, allocator);
                switch (lres) {
                    .Number => |lval| {
                        switch (rres) {
                            .Number => |rval| {
                                switch (x.op.token_type) {
                                    TokenType.TokPlus => return ResultType{ .Number = lval + rval },
                                    TokenType.TokMinus => return ResultType{ .Number = lval - rval },
                                    TokenType.TokStar => return ResultType{ .Number = lval * rval },
                                    TokenType.TokSlash => return ResultType{ .Number = lval / rval },
                                    TokenType.TokMod => return ResultType{ .Number = @mod(lval, rval) },
                                    TokenType.TokCaret => return ResultType{ .Number = pow(f64, lval, rval) },
                                    TokenType.TokEq => return ResultType{ .Bool = lval == rval },
                                    TokenType.TokNe => return ResultType{ .Bool = lval != rval },
                                    TokenType.TokLt => return ResultType{ .Bool = lval < rval },
                                    TokenType.TokLe => return ResultType{ .Bool = lval <= rval },
                                    TokenType.TokGt => return ResultType{ .Bool = lval > rval },
                                    TokenType.TokGe => return ResultType{ .Bool = lval >= rval },
                                    else => unreachable,
                                }
                            },
                            .Bool => |rval| {
                                const new_rval: f64 = if (rval) 1.0 else 0.0;
                                switch (x.op.token_type) {
                                    TokenType.TokEq => return ResultType{ .Bool = lval == new_rval },
                                    TokenType.TokNe => return ResultType{ .Bool = lval != new_rval },
                                    TokenType.TokPlus => return ResultType{ .Number = lval + new_rval },
                                    TokenType.TokMinus => return ResultType{ .Number = lval - new_rval },
                                    TokenType.TokStar => return ResultType{ .Number = lval * new_rval },
                                    TokenType.TokSlash => return ResultType{ .Number = lval / new_rval },
                                    else => unreachable,
                                }
                            },
                            else => unreachable,
                        }
                    },
                    .Bool => |lval| {
                        switch (rres) {
                            .Bool => |rval| {
                                if (x.op.token_type == TokenType.TokEq) return ResultType{ .Bool = lval == rval };
                                if (x.op.token_type == TokenType.TokNe) return ResultType{ .Bool = lval != rval };
                            },
                            .Number => |rval| {
                                const new_lval: f64 = if (lval) 1.0 else 0.0;
                                if (x.op.token_type == TokenType.TokEq) return ResultType{ .Bool = new_lval == rval };
                                if (x.op.token_type == TokenType.TokNe) return ResultType{ .Bool = new_lval != rval };
                                if (x.op.token_type == TokenType.TokPlus) return ResultType{ .Number = new_lval + rval };
                                if (x.op.token_type == TokenType.TokMinus) return ResultType{ .Number = new_lval - rval };
                                if (x.op.token_type == TokenType.TokStar) return ResultType{ .Number = new_lval * rval };
                                if (x.op.token_type == TokenType.TokSlash) return ResultType{ .Number = new_lval / rval };
                            },
                            else => unreachable,
                        }
                    },
                    .String => |lval| {
                        switch (rres) {
                            .String => |rval| {
                                if (x.op.token_type == TokenType.TokEq) return ResultType{ .Bool = strEql(u8, lval, rval) };
                                if (x.op.token_type == TokenType.TokNe) return ResultType{ .Bool = !strEql(u8, lval, rval) };
                                if (x.op.token_type == TokenType.TokPlus) {
                                    const result = try allocPrint(allocator, "{s}{s}", .{ lval, rval });
                                    return ResultType{ .String = result };
                                }
                            },
                            .Number => |rval| {
                                if (x.op.token_type == TokenType.TokPlus) {
                                    const result = try allocPrint(allocator, "{s}{d}", .{ lval, rval });
                                    return ResultType{ .String = result };
                                }
                                if (x.op.token_type == TokenType.TokStar) {
                                    const rounded_rval = @as(usize, @intFromFloat(rval));
                                    const result = try allocator.alloc(u8, rounded_rval * lval.len);
                                    for (0..rounded_rval) |i| {
                                        strCopy(u8, result[i * lval.len ..], lval);
                                    }
                                    return ResultType{ .String = result };
                                }
                            },
                            else => unreachable,
                        }
                    },
                }
            },
        }

        unreachable;
    }
};
