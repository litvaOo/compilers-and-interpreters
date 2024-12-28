const Expression = @import("model.zig").Expression;
const TokenType = @import("tokens.zig").TokenType;
const strEql = @import("std").mem.eql;
const strCopy = @import("std").mem.copyForwards;
pub const ResultType = union(enum) {
    Bool: bool,
    Number: f64,
    String: []const u8,
};

pub const Interpreter = struct {
    pub fn interpret(self: *Interpreter, node: Expression) ResultType {
        switch (node) {
            .Integer => |x| return ResultType{ .Number = @floatFromInt(x.value) },
            .Float => |x| return ResultType{ .Number = x.value },
            .Bool => |x| return ResultType{ .Bool = x.value },
            .String => |x| return ResultType{ .String = x.value },
            .Grouping => |x| return self.interpret(x.value.*),
            .UnaryOp => |x| {
                const rres = self.interpret(x.exp.*);
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
                const lres = self.interpret(x.left.*);
                switch (lres) {
                    .Bool => |lval| {
                        if (x.op.token_type == TokenType.TokOr and lval == true) return ResultType{ .Bool = true };
                        if (x.op.token_type == TokenType.TokAnd and lval == false) return ResultType{ .Bool = false };
                        return self.interpret(x.right.*);
                    },
                    else => unreachable,
                }
            },
            .BinOp => |x| {
                const lres = self.interpret(x.left.*);
                const rres = self.interpret(x.right.*);
                switch (lres) {
                    // .Number => |lval| {},
                    // .Bool => |lval| {},
                    .String => |lval| {
                        switch (rres) {
                            .String => |rval| {
                                if (x.op.token_type == TokenType.TokEq) return ResultType{ .Bool = strEql(u8, lval, rval) };
                                if (x.op.token_type == TokenType.TokNe) return ResultType{ .Bool = !strEql(u8, lval, rval) };
                                if (x.op.token_type == TokenType.TokPlus) {
                                    const result: [lval.len + rval.len]u8 = undefined;
                                    strCopy(u8, result[0..], lval);
                                    strCopy(u8, result[lval.len..], rval);
                                    return ResultType{ .String = result };
                                }
                            },
                            else => unreachable,
                        }
                    },
                    else => unreachable,
                }
            },
        }

        unreachable;
    }
};
