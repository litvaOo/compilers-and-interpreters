const std = @import("std");

pub const Token = struct {
    token_type: TokenType,
    lexeme: []const u8,
    line: u32,
    line_position: u32,

    pub fn format(self: Token, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("Type: {} lexeme {s} on line at {} position {}", .{ self.token_type, self.lexeme, self.line, self.line_position });
    }
};

pub const TokenType = enum {
    TokLparen,
    TokRparen,
    TokLcurly,
    TokRcurly,
    TokLsquar,
    TokRsquar,
    TokComma,
    TokDot,
    TokPlus,
    TokMinus,
    TokStar,
    TokSlash,
    TokCaret,
    TokMod,
    TokColon,
    TokSemicolon,
    TokQuestion,
    TokNot,
    TokGt,
    TokLt,
    TokGe,
    TokLe,
    TokNe,
    TokEq,
    TokAssign,
    TokGtgt,
    TokLtlt,
    TokIdentifier,
    TokString,
    TokInteger,
    TokFloat,
    TokIf,
    TokThen,
    TokElse,
    TokTrue,
    TokFalse,
    TokAnd,
    TokOr,
    TokWhile,
    TokDo,
    TokFor,
    TokFunc,
    TokNull,
    TokEnd,
    TokPrint,
    TokPrintln,
    TokRet,
    TokLocal,
};

pub var KeyWords: std.StringHashMap(TokenType) = undefined;

pub fn init_keywords(allocator: anytype) !void {
    KeyWords = std.StringHashMap(TokenType).init(allocator);
    try KeyWords.put("if", TokenType.TokIf);
    try KeyWords.put("else", TokenType.TokElse);
    try KeyWords.put("then", TokenType.TokThen);
    try KeyWords.put("true", TokenType.TokTrue);
    try KeyWords.put("false", TokenType.TokFalse);
    try KeyWords.put("and", TokenType.TokAnd);
    try KeyWords.put("or", TokenType.TokOr);
    try KeyWords.put("while", TokenType.TokWhile);
    try KeyWords.put("do", TokenType.TokDo);
    try KeyWords.put("for", TokenType.TokFor);
    try KeyWords.put("func", TokenType.TokFunc);
    try KeyWords.put("null", TokenType.TokNull);
    try KeyWords.put("end", TokenType.TokEnd);
    try KeyWords.put("print", TokenType.TokPrint);
    try KeyWords.put("println", TokenType.TokPrintln);
    try KeyWords.put("ret", TokenType.TokRet);
    try KeyWords.put("local", TokenType.TokLocal);
}

pub fn token_init(token_type: TokenType, lexeme: []const u8, line: u32, line_position: u32) Token {
    return Token{ .token_type = token_type, .lexeme = lexeme, .line = line, .line_position = line_position };
}
