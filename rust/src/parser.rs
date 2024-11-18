use core::panic;

use crate::model::Expression;
use crate::token::{Token, TokenType};

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Parser {
        Parser { tokens, current: 0 }
    }

    fn advance(&mut self) -> Option<Token> {
        if self.current < self.tokens.len() {
            let token = self.tokens[self.current].clone();
            self.current += 1;
            return Some(token);
        }
        None
    }

    fn peek(&self) -> Option<Token> {
        if self.current < self.tokens.len() {
            return Some(self.tokens[self.current].clone());
        }
        None
    }

    fn is_next(&self, expected_type: TokenType) -> bool {
        match self.peek() {
            Some(token) => token.token_type == expected_type,
            None => false,
        }
    }

    fn expect(&mut self, expected_type: TokenType) -> Token {
        match self.peek() {
            Some(token) => {
                if token.token_type == expected_type {
                    return self.advance().unwrap();
                }
                panic!("Unexpected token")
            }
            None => panic!("Unexpected token"),
        }
    }

    fn match_token(&mut self, token_type: TokenType) -> bool {
        match self.peek() {
            Some(token) => {
                if token.token_type == token_type {
                    self.current += 1;
                    return true;
                }
                false
            }
            None => false,
        }
    }

    fn previous_token(&mut self) -> Option<Token> {
        if self.current >= 1 {
            return Some(self.tokens[self.current - 1].clone());
        }
        None
    }

    fn term(&mut self) -> Expression {
        println!("Inside term");
        let expr = self.factor();
        if self.match_token(TokenType::TokStar) || self.match_token(TokenType::TokSlash) {
            match self.previous_token() {
                Some(op) => {
                    return Expression::BinOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.factor()),
                    }
                }
                None => panic!("Couldn't get previous_token"),
            };
        }
        expr
    }

    fn expr(&mut self) -> Expression {
        println!("Inside expr");
        let expr = self.term();
        if self.match_token(TokenType::TokPlus) || self.match_token(TokenType::TokMinus) {
            println!("Matched to Plus or minus");
            match self.previous_token() {
                Some(op) => {
                    return Expression::BinOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.term()),
                    }
                }
                None => panic!("Failed to get a previous_token"),
            };
        }
        expr
    }

    fn primary(&mut self) -> Expression {
        println!("Inside primary");
        if self.match_token(TokenType::TokInteger) {
            println!("Matched integer");
            match self.previous_token() {
                Some(token) => {
                    return Expression::Integer {
                        value: token.lexeme.parse::<i64>().unwrap(),
                    }
                }
                None => panic!("Failed getting Integer"),
            };
        };
        if self.match_token(TokenType::TokFloat) {
            println!("Matched float");
            match self.previous_token() {
                Some(token) => {
                    return Expression::Float {
                        value: token.lexeme.parse::<f64>().unwrap(),
                    }
                }
                None => panic!("Failed getting Float"),
            };
        };
        if self.match_token(TokenType::TokLparen) {
            println!("Matched parentheses");
            let expr = self.expr();
            if self.match_token(TokenType::TokRparen) {
                return Expression::Grouping {
                    value: Box::new(expr),
                };
            }
            panic!("Unable to find closing parentheses");
        }
        panic!("Unable to parse primary token")
    }

    fn unary(&mut self) -> Expression {
        println!("Inside unary");
        if self.match_token(TokenType::TokNot)
            || self.match_token(TokenType::TokMinus)
            || self.match_token(TokenType::TokPlus)
        {
            match self.previous_token() {
                Some(token) => {
                    return Expression::UnaryOp {
                        op: token,
                        exp: Box::new(self.unary()),
                    }
                }
                None => panic!("No previous_token"),
            };
        };
        self.primary()
    }

    fn factor(&mut self) -> Expression {
        println!("Inside factory");
        self.unary()
    }

    pub fn parse(&mut self) -> Expression {
        self.expr()
    }
}
