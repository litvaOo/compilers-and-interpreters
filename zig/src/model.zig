const Token = @import("tokens.zig").Token;
const TypeInfo = @import("std").TypeInfo;
const std = @import("std");

pub const Statements = std.ArrayList(Statement);

pub const Statement = union(enum) {
    PrintStatement: struct { value: Expression },
    PrintlnStatement: struct { value: Expression },
    IfStatement: struct { test_expr: Expression, then_stmts: Statements, else_stmts: Statements },
    Assignment: struct { left: Expression, right: Expression },
};

pub const Expression = union(enum) {
    Integer: struct { value: i64 },
    Float: struct { value: f64 },
    Bool: struct { value: bool },
    String: struct { value: []const u8 },
    UnaryOp: struct { op: Token, exp: *Expression },
    BinOp: struct { op: Token, left: *Expression, right: *Expression },
    Grouping: struct { value: *Expression },
    LogicalOp: struct { op: Token, left: *Expression, right: *Expression },
    Identifier: struct { name: []const u8 },

    pub fn format(self: Expression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        switch (self) {
            .Integer => |x| try writer.print("{any}", .{x.value}),
            .Float => |x| try writer.print("{any}", .{x.value}),
            .UnaryOp => |x| try writer.print("{s} ({})", .{ x.op.lexeme, x.exp.* }),
            .BinOp => |x| try writer.print("{s} ({}, {})", .{ x.op.lexeme, x.left.*, x.right.* }),
            .Grouping => |x| try writer.print("({any})", .{x.value}),
            .Bool => |x| try writer.print("({any})", .{x.value}),
            .String => |x| try writer.print("({s})", .{x.value}),
            .LogicalOp => |x| try writer.print("{s} ({}, {})", .{ x.op.lexeme, x.left.*, x.right.* }),
            .Identifier => |x| try writer.print("{s}", .{x.name}),
        }
    }
};

pub const Node = union(enum) {
    Expr: Expression,
    Stmt: Statement,
    Stmts: Statements,
};
