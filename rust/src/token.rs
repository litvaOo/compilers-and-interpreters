use std::collections::HashMap;
use std::fmt::{self, Display, Formatter};

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String,
    line: u32,
    position: u32,
}

impl Display for Token {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(
            f,
            "({:?}, {}, on line {} starting at {})",
            self.token_type, self.lexeme, self.line, self.position
        )
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
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

pub fn token_init(token_type: TokenType, lexeme: String, line: u32, position: u32) -> Token {
    Token {
        token_type,
        lexeme,
        line,
        position,
    }
}
