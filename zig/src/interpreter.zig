const Expression = @import("model.zig").Expression;
const Node = @import("model.zig").Node;
const Statement = @import("model.zig").Statement;
const Statements = @import("model.zig").Statements;
const TokenType = @import("tokens.zig").TokenType;
const strEql = @import("std").mem.eql;
const strCopy = @import("std").mem.copyForwards;
const Allocator = @import("std").mem.Allocator;
const std = @import("std");
const allocPrint = @import("std").fmt.allocPrint;
const pow = @import("std").math.pow;
const State = @import("state.zig").State;
const state_init = @import("state.zig").state_init;

pub const ResultType = union(enum) {
    Bool: bool,
    Number: f64,
    String: []const u8,
    Null,

    pub fn format(self: ResultType, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        switch (self) {
            .Bool => |x| try writer.print("{}", .{x}),
            .Number => |x| try writer.print("{d}", .{x}),
            .String => |x| {
                var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
                defer arena.deinit();
                const allocator = arena.allocator();
                const str = try std.fmt.allocPrint(allocator, "\"{s}\"", .{x});
                const decoded_buf = try std.zig.string_literal.parseAlloc(allocator, str);
                try writer.print("{s}", .{decoded_buf});
                allocator.free(decoded_buf);
            },
            .Null => try writer.print("", .{}),
        }
    }
};

pub const Interpreter = struct {
    allocator: std.mem.Allocator,

    pub fn interpret_ast(self: *Interpreter, node: Node) !ResultType {
        var state = state_init(null, self.allocator);
        _ = &state;
        return try self.interpret(node, &state);
    }

    pub fn interpret(self: *Interpreter, node: Node, state: *State) !ResultType {
        switch (node) {
            .Stmt => |stmt| {
                switch (stmt) {
                    .PrintStatement => |val| std.debug.print("{s}", .{try self.interpret(Node{ .Expr = val.value }, state)}),
                    .PrintlnStatement => |val| std.debug.print("{s}\n", .{try self.interpret(Node{ .Expr = val.value }, state)}),
                    .Assignment => |val| {
                        const rres = try self.interpret(Node{ .Expr = val.right }, state);
                        switch (val.left) {
                            .Identifier => |left| {
                                // std.debug.print("Found Identifier {s}, writing into hashtable", .{left.name});
                                try state.set_item(left.name, rres);
                                return ResultType.Null;
                            },
                            else => unreachable,
                        }
                    },
                    .IfStatement => |data| {
                        const express = try self.interpret(Node{ .Expr = data.test_expr }, state);
                        var new_state = state.get_child_env();
                        _ = &new_state;
                        switch (express) {
                            .Number => |num| {
                                if (num != 0.0) {
                                    return self.interpret(Node{ .Stmts = data.then_stmts }, &new_state);
                                }
                                return self.interpret(Node{ .Stmts = data.else_stmts }, &new_state);
                            },
                            .Bool => |val| {
                                if (val) {
                                    return self.interpret(Node{ .Stmts = data.then_stmts }, &new_state);
                                }
                                return self.interpret(Node{ .Stmts = data.else_stmts }, &new_state);
                            },
                            .String => |val| {
                                if (val.len != 0) {
                                    return self.interpret(Node{ .Stmts = data.then_stmts }, &new_state);
                                }
                                return self.interpret(Node{ .Stmts = data.else_stmts }, &new_state);
                            },
                            .Null => unreachable,
                        }
                    },
                }
                return ResultType.Null;
            },
            .Stmts => |stmts| {
                for (stmts.items) |stmt| {
                    _ = try self.interpret(Node{ .Stmt = stmt }, state);
                }
                return ResultType.Null;
            },
            .Expr => |express| {
                switch (express) {
                    .Identifier => |x| {
                        const res = state.get_item(x.name);
                        if (res == null) {
                            unreachable;
                        }
                        return res.?;
                    },
                    .Integer => |x| return ResultType{ .Number = @floatFromInt(x.value) },
                    .Float => |x| return ResultType{ .Number = x.value },
                    .Bool => |x| return ResultType{ .Bool = x.value },
                    .String => |x| return ResultType{ .String = x.value },
                    .Grouping => |x| return try self.interpret(Node{ .Expr = x.value.* }, state),
                    .UnaryOp => |x| {
                        const rres = try self.interpret(Node{ .Expr = x.exp.* }, state);
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
                        const lres = try self.interpret(Node{ .Expr = x.left.* }, state);
                        switch (lres) {
                            .Bool => |lval| {
                                if (x.op.token_type == TokenType.TokOr and lval == true) return ResultType{ .Bool = true };
                                if (x.op.token_type == TokenType.TokAnd and lval == false) return ResultType{ .Bool = false };
                                return try self.interpret(Node{ .Expr = x.right.* }, state);
                            },
                            else => unreachable,
                        }
                    },
                    .BinOp => |x| {
                        const lres = try self.interpret(Node{ .Expr = x.left.* }, state);
                        const rres = try self.interpret(Node{ .Expr = x.right.* }, state);
                        // std.debug.print("{s} {s}\n", .{ x.left.*, x.right.* });
                        switch (lres) {
                            .Null => unreachable,
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
                                            const result = try allocPrint(self.allocator, "{s}{s}", .{ lval, rval });
                                            return ResultType{ .String = result };
                                        }
                                    },
                                    .Number => |rval| {
                                        if (x.op.token_type == TokenType.TokPlus) {
                                            const result = try allocPrint(self.allocator, "{s}{d}", .{ lval, rval });
                                            return ResultType{ .String = result };
                                        }
                                        if (x.op.token_type == TokenType.TokStar) {
                                            const rounded_rval = @as(usize, @intFromFloat(rval));
                                            const result = try self.allocator.alloc(u8, rounded_rval * lval.len);
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
            },
        }

        unreachable;
    }
};
