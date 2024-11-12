use std::collections::HashMap;
use std::fmt::{self, Display, Formatter};

#[derive(Debug)]
pub struct Token {
    token_type: TokenType,
    lexeme: String,
    line: u32,
}

impl Display for Token {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "({:?}, {}, {})", self.token_type, self.lexeme, self.line)
    }
}

#[derive(Debug, Clone, Copy)]
pub enum TokenType {
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
}

thread_local! {
    pub static KEYWORDS: HashMap<&'static str, TokenType> = HashMap::from([
        ("if", TokenType::TokIf),
        ("else", TokenType::TokElse),
        ("then", TokenType::TokThen),
        ("true", TokenType::TokTrue),
        ("false", TokenType::TokFalse),
        ("and", TokenType::TokAnd),
        ("or", TokenType::TokOr),
        ("while", TokenType::TokWhile),
        ("do", TokenType::TokDo),
        ("for", TokenType::TokFor),
        ("func", TokenType::TokFunc),
        ("null", TokenType::TokNull),
        ("end", TokenType::TokEnd),
        ("print", TokenType::TokPrint),
        ("println", TokenType::TokPrintln),
        ("ret", TokenType::TokRet),
    ]);
}

pub fn token_init(token_type: TokenType, lexeme: String, line: u32) -> Token {
    Token {
        token_type,
        lexeme,
        line,
    }
}
