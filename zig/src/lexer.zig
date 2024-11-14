const std = @import("std");
const tokens = @import("tokens.zig");
const ArrayList = std.ArrayList;
const ascii = std.ascii;

pub const Lexer = struct {
    tokens: ArrayList(tokens.Token),
    start: u32,
    curr: u32,
    line: u32,
    line_position: u32,
    source: []const u8,

    fn advance(self: *Lexer) u8 {
        if (self.curr >= self.source.len) {
            return ' ';
        }
        const ch = self.source[self.curr];
        self.curr += 1;
        self.line_position += 1;
        return ch;
    }

    fn add_token(self: *Lexer, token_type: tokens.TokenType) !void {
        try self.tokens.append(tokens.token_init(token_type, self.source[self.start..self.curr], self.line, self.line_position));
    }

    fn peek(self: *Lexer) u8 {
        if (self.curr >= self.source.len) {
            return ' ';
        }
        return self.source[self.curr];
    }

    fn lookahead(self: *Lexer) u8 {
        if (self.curr + 1 >= self.source.len) {
            return ' ';
        }
        return self.source[self.curr + 1];
    }

    fn match_char(self: *Lexer, expected: u8) bool {
        if ((self.curr >= self.source.len) or (self.source[self.curr] != expected)) {
            return false;
        }
        self.curr += 1;
        return true;
    }

    fn handle_number(self: *Lexer) !void {
        while (ascii.isDigit(self.peek())) {
            _ = self.advance();
        }
        if ((self.peek() == '.') and (ascii.isDigit(self.lookahead()))) {
            _ = self.advance();
            while (ascii.isDigit(self.peek())) {
                _ = self.advance();
            }
            try self.add_token(tokens.TokenType.TokFloat);
        } else {
            try self.add_token(tokens.TokenType.TokInteger);
        }
    }

    fn handle_string(self: *Lexer, quote: u8) !void {
        while ((self.peek() != quote) and (self.curr < self.source.len)) {
            _ = self.advance();
        }
        if (self.curr >= self.source.len) {
            std.debug.panic("Unfinished string at {}", .{self.line});
        }
        _ = self.advance();
        try self.add_token(tokens.TokenType.TokString);
    }

    fn handle_identifier(self: *Lexer) !void {
        while ((ascii.isAlphanumeric(self.peek())) or self.peek() == '_') {
            _ = self.advance();
        }
        try self.add_token(tokens.KeyWords.get(self.source[self.start..self.curr]) orelse tokens.TokenType.TokIdentifier);
    }

    pub fn tokenize(self: *Lexer) !void {
        while (self.curr < self.source.len) {
            self.start = self.curr;
            const ch = self.advance();
            switch (ch) {
                '\n' => {
                    self.line += 1;
                    self.line_position = 1;
                },
                '#' => {
                    while ((self.peek() != '\n') and (self.curr < self.source.len)) {
                        _ = self.advance();
                    }
                },
                '(' => try self.add_token(tokens.TokenType.TokLparen),
                ')' => try self.add_token(tokens.TokenType.TokRparen),
                '{' => try self.add_token(tokens.TokenType.TokLcurly),
                '}' => try self.add_token(tokens.TokenType.TokRcurly),
                '[' => try self.add_token(tokens.TokenType.TokLsquar),
                ']' => try self.add_token(tokens.TokenType.TokRsquar),
                ',' => try self.add_token(tokens.TokenType.TokComma),
                '.' => try self.add_token(tokens.TokenType.TokDot),
                '+' => try self.add_token(tokens.TokenType.TokPlus),
                '-' => {
                    if (self.match_char('-')) {
                        while (self.peek() != '\n' and self.curr < self.source.len) {
                            _ = self.advance();
                        }
                    } else {
                        try self.add_token(tokens.TokenType.TokMinus);
                    }
                },
                '*' => try self.add_token(tokens.TokenType.TokStar),
                '/' => try self.add_token(tokens.TokenType.TokSlash),

                '^' => try self.add_token(tokens.TokenType.TokCaret),
                '%' => try self.add_token(tokens.TokenType.TokMod),
                ':' => {
                    if (self.match_char('=')) {
                        try self.add_token(tokens.TokenType.TokAssign);
                    } else {
                        try self.add_token(tokens.TokenType.TokColon);
                    }
                },
                ';' => try self.add_token(tokens.TokenType.TokSemicolon),
                '?' => try self.add_token(tokens.TokenType.TokQuestion),
                '>' => {
                    if (self.match_char('=')) {
                        try self.add_token(tokens.TokenType.TokGe);
                    } else if (self.match_char('>')) {
                        try self.add_token(tokens.TokenType.TokGtgt);
                    } else {
                        try self.add_token(tokens.TokenType.TokGt);
                    }
                },
                '<' => {
                    if (self.match_char('=')) {
                        try self.add_token(tokens.TokenType.TokLe);
                    } else if (self.match_char('<')) {
                        try self.add_token(tokens.TokenType.TokLtlt);
                    } else {
                        try self.add_token(tokens.TokenType.TokLt);
                    }
                },
                '=' => {
                    if (self.match_char('=')) {
                        try self.add_token(tokens.TokenType.TokEq);
                    }
                },
                '~' => {
                    if (self.match_char('=')) {
                        try self.add_token(tokens.TokenType.TokNe);
                    } else {
                        try self.add_token(tokens.TokenType.TokNot);
                    }
                },
                '"' => try self.handle_string(ch),
                '\'' => try self.handle_string(ch),
                else => {
                    if (ascii.isDigit(ch)) {
                        try self.handle_number();
                    }
                    if (ascii.isAlphanumeric(ch)) {
                        try self.handle_identifier();
                    }
                },
            }
        }
    }
};

pub fn init_lexer(allocator: anytype, source: []const u8) !Lexer {
    try tokens.init_keywords(allocator);
    return .{ .tokens = ArrayList(tokens.Token).init(allocator), .start = 0, .curr = 0, .line = 1, .line_position = 1, .source = source };
}
