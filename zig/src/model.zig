const Token = @import("tokens.zig").Token;
const TypeInfo = @import("std").TypeInfo;
const std = @import("std");

pub const Expression = union(enum) {
    Integer: struct { value: i64 },
    Float: struct { value: f64 },
    Bool: struct { value: bool },
    String: struct { value: []const u8 },
    UnaryOp: struct { op: Token, exp: *Expression },
    BinOp: struct { op: Token, left: *Expression, right: *Expression },
    Grouping: struct { value: *Expression },
    LogicalOp: struct { op: Token, left: *Expression, right: *Expression },

    pub fn format(self: Expression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        switch (self) {
            .Integer => |x| try writer.print("{}", .{x.value}),
            .Float => |x| try writer.print("{}", .{x.value}),
            .UnaryOp => |x| try writer.print("{s} ({})", .{ x.op.lexeme, x.exp.* }),
            .BinOp => |x| try writer.print("{s} ({}, {})", .{ x.op.lexeme, x.left.*, x.right.* }),
            .Grouping => |x| try writer.print("({})", .{x.value}),
            .Bool => |x| try writer.print("({})", .{x.value}),
            .String => |x| try writer.print("({})", .{x.value}),
            .LogicalOp => |x| try writer.print("{s} ({}, {})", .{ x.op.lexeme, x.left.*, x.right.* }),
        }
    }
};
