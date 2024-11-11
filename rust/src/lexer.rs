use core::panic;

use crate::token::{token_init, TokenType};

#[derive(Debug)]
pub struct LexerImpl {
    pub tokens: Vec<crate::token::Token>,
    start: usize,
    curr: usize,
    line: u32,
    source: String,
}

pub fn lexer_init(source: String) -> LexerImpl {
    LexerImpl {
        tokens: vec![],
        start: 0,
        curr: 0,
        line: 1,
        source,
    }
}

pub trait Lexer {
    fn tokenize(&mut self);
    fn advance(&mut self) -> char;
    fn add_token(&mut self, token_type: TokenType);
    fn peek(&self) -> char;
    fn lookahead(&self, n: usize) -> char;
    fn match_char(&mut self, expected: char) -> bool;
    fn handle_number(&mut self);
    fn handle_string(&mut self, quote: char);
    fn handle_identifier(&mut self);
}

impl Lexer for LexerImpl {
    fn advance(&mut self) -> char {
        if self.curr >= self.source.len() {
            return '\0';
        }
        let ch = self.source.chars().nth(self.curr);
        self.curr += 1;
        ch.unwrap()
    }

    fn add_token(&mut self, token_type: TokenType) {
        self.tokens.push(token_init(
            token_type,
            self.source[self.start..self.curr].to_owned(),
            self.line,
        ));
    }

    fn peek(&self) -> char {
        if self.curr >= self.source.len() {
            return '\0';
        }
        self.source.chars().nth(self.curr).unwrap()
    }

    fn lookahead(&self, n: usize) -> char {
        if self.curr + n >= self.source.len() {
            return '\0';
        }
        self.source.chars().nth(self.curr + n).unwrap()
    }

    fn match_char(&mut self, expected: char) -> bool {
        if self.curr >= self.source.len() || self.peek() != expected {
            return false;
        }
        self.curr += 1;
        true
    }

    fn handle_number(&mut self) {
        while self.peek().is_ascii_digit() {
            self.advance();
        }
        if self.peek() == '.' && self.lookahead(1).is_ascii_digit() {
            self.advance();
            while self.peek().is_ascii_digit() {
                self.advance();
            }
            self.add_token(TokenType::TokFloat);
        } else {
            self.add_token(TokenType::TokInteger);
        }
    }

    fn handle_string(&mut self, quote: char) {
        while self.peek() != quote && self.curr < self.source.len() {
            self.advance();
        }
        if self.curr >= self.source.len() {
            panic!("Unfinished string !{}", self.line);
        }
        self.advance();
        self.add_token(TokenType::TokString);
    }

    fn handle_identifier(&mut self) {
        while self.peek().is_alphanumeric() || self.peek() == '_' {
            self.advance();
        }
        self.add_token(crate::token::KEYWORDS.with(|keyword| {
            *keyword
                .get(&self.source[self.start..self.curr])
                .unwrap_or(&TokenType::TokIdentifier)
        }));
    }

    fn tokenize(&mut self) {
        while self.curr < self.source.len() {
            self.start = self.curr;
            let ch = self.advance();
            match ch {
                '\n' => self.line += 1,
                '#' => {
                    while self.peek() != '\n' && self.curr < self.source.len() {
                        self.advance();
                    }
                }
                '(' => self.add_token(TokenType::TokLparen),
                ')' => self.add_token(TokenType::TokRparen),
                '{' => self.add_token(TokenType::TokLcurly),
                '}' => self.add_token(TokenType::TokRcurly),
                '[' => self.add_token(TokenType::TokLsquar),
                ']' => self.add_token(TokenType::TokRsquar),
                ',' => self.add_token(TokenType::TokComma),
                '.' => self.add_token(TokenType::TokDot),
                '+' => self.add_token(TokenType::TokPlus),
                '-' => {
                    if self.match_char('-') {
                        while self.peek() != '\n' && self.curr < self.source.len() {
                            self.advance();
                        }
                    } else {
                        self.add_token(TokenType::TokMinus);
                    }
                }
                '*' => self.add_token(TokenType::TokStar),
                '/' => self.add_token(TokenType::TokSlash),

                '^' => self.add_token(TokenType::TokCaret),
                '%' => self.add_token(TokenType::TokMod),
                ':' => {
                    if self.match_char('=') {
                        self.add_token(TokenType::TokAssign);
                    } else {
                        self.add_token(TokenType::TokColon);
                    }
                }
                ';' => self.add_token(TokenType::TokSemicolon),
                '?' => self.add_token(TokenType::TokQuestion),
                '>' => {
                    if self.match_char('=') {
                        self.add_token(TokenType::TokGe);
                    } else if self.match_char('>') {
                        self.add_token(TokenType::TokGtgt);
                    } else {
                        self.add_token(TokenType::TokGt);
                    }
                }
                '<' => {
                    if self.match_char('=') {
                        self.add_token(TokenType::TokLe);
                    } else if self.match_char('<') {
                        self.add_token(TokenType::TokLtlt);
                    } else {
                        self.add_token(TokenType::TokLt)
                    }
                }
                '=' => {
                    if self.match_char('=') {
                        self.add_token(TokenType::TokEq);
                    }
                }
                '~' => {
                    if self.match_char('=') {
                        self.add_token(TokenType::TokNe)
                    } else {
                        self.add_token(TokenType::TokNot);
                    }
                }
                '"' => self.handle_string(ch),
                '\'' => self.handle_string(ch),
                _ => {
                    if ch.is_ascii_digit() {
                        self.handle_number()
                    }
                    if ch.is_alphanumeric() {
                        self.handle_identifier()
                    }
                }
            }
        }
    }
}
