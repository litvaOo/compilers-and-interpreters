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
    Return: *ResultType,
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
            .Return => |x| try writer.print("returning {}", .{x}),
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
                    .PrintStatement => |val| try std.io.getStdOut().writer().print("{s}", .{try self.interpret(Node{ .Expr = val.value }, state)}),
                    .PrintlnStatement => |val| try std.io.getStdOut().writer().print("{s}\n", .{try self.interpret(Node{ .Expr = val.value }, state)}),
                    .Assignment => |val| {
                        const rres = try self.interpret(Node{ .Expr = val.right }, state);
                        switch (val.left) {
                            .Identifier => |left| {
                                try state.set_item(left.name, rres);
                                return ResultType.Null;
                            },
                            else => unreachable,
                        }
                    },
                    .LocalAssignment => |val| {
                        const rres = try self.interpret(Node{ .Expr = val.right }, state);
                        switch (val.left) {
                            .Identifier => |left| {
                                try state.set_local_item(left.name, rres);
                                return ResultType.Null;
                            },
                            else => unreachable,
                        }
                    },
                    .Return => |ret| {
                        const result = try self.allocator.create(ResultType);
                        result.* = try self.interpret(Node{ .Expr = ret.val }, state);
                        return ResultType{ .Return = result };
                    },
                    .FunctionDeclaration => |func| {
                        try state.set_func(func.name, stmt);
                        return ResultType.Null;
                    },
                    .FunctionCall => |call| return try self.interpret(Node{ .Expr = call.expr }, state),
                    .Parameter => return ResultType.Null,
                    .While => |data| {
                        var new_state = state.get_child_env();
                        while (true) {
                            const test_res = try self.interpret(Node{ .Expr = data.test_expr }, &new_state);
                            switch (test_res) {
                                .Number => |val| if (val == 0.0) break,
                                .Bool => |val| if (!val) break,
                                .String => |val| if (val.len == 0) break,
                                .Return => return test_res,
                                .Null => unreachable,
                            }
                            const res = try self.interpret(Node{ .Stmts = data.stmts }, &new_state);
                            switch (res) {
                                .Return => return res,
                                else => continue,
                            }
                        }
                        return ResultType.Null;
                    },
                    .For => |data| {
                        var new_state = state.get_child_env();
                        switch (data.id) {
                            .Identifier => |id| {
                                const start_res = try self.interpret(Node{ .Expr = data.start }, &new_state);
                                switch (start_res) {
                                    .Number => |start_val| {
                                        try new_state.set_item(id.name, start_res);
                                        const end_res = try self.interpret(Node{ .Expr = data.end }, &new_state);
                                        switch (end_res) {
                                            .Number => |end_val| {
                                                const step_res = try self.interpret(Node{ .Expr = data.step }, &new_state);
                                                switch (step_res) {
                                                    .Number => |step_val| {
                                                        while (true) {
                                                            const current_res = &new_state.get_item(id.name).?;
                                                            switch (current_res.*) {
                                                                .Number => |current_val| {
                                                                    if ((start_val <= end_val and current_val >= end_val) or (start_val >= end_val and current_val <= end_val)) {
                                                                        break;
                                                                    }
                                                                    const res = try self.interpret(Node{ .Stmts = data.stmts }, &new_state);
                                                                    switch (res) {
                                                                        .Return => return res,
                                                                        else => {
                                                                            try new_state.set_item(id.name, ResultType{ .Number = current_val + step_val });
                                                                        },
                                                                    }
                                                                },
                                                                else => unreachable,
                                                            }
                                                        }
                                                    },
                                                    else => unreachable,
                                                }
                                            },
                                            else => unreachable,
                                        }
                                    },
                                    else => unreachable,
                                }
                            },
                            else => unreachable,
                        }
                        return ResultType.Null;
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
                            .Return => return express,
                            .Null => unreachable,
                        }
                    },
                }
                return ResultType.Null;
            },
            .Stmts => |stmts| {
                for (stmts.items) |stmt| {
                    const res = try self.interpret(Node{ .Stmt = stmt }, state);
                    switch (res) {
                        .Return => return res,
                        else => continue,
                    }
                }
                return ResultType.Null;
            },
            .Expr => |express| {
                switch (express) {
                    .FunctionCall => |func_call| {
                        const func = state.get_func(func_call.name).?;
                        if (func.FunctionDeclaration.params.items.len != func_call.args.items.len) {
                            std.debug.print("Wrong amount of params", .{});
                            unreachable;
                        }
                        const params = func.FunctionDeclaration.params.items;
                        const args = func_call.args.items;
                        var func_state = state.get_child_env();
                        for (params, args) |param, arg| {
                            try func_state.set_local_item(param.Parameter.name, try self.interpret(Node{ .Expr = arg }, state));
                        }

                        const res = try self.interpret(Node{ .Stmts = func.FunctionDeclaration.stmts }, &func_state);
                        switch (res) {
                            .Return => |ret| return ret.*,
                            else => return res,
                        }
                    },
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
                            .Return => unreachable,
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
