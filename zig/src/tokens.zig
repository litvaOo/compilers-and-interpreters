const Token = {
    token_type: []const u8,
    lexeme: []const u8
}

fn token_init(token_type: []const u8, lexeme: []const u8) Token {
    return Token{.token_type: token_type, .lexeme: lexeme};
}
