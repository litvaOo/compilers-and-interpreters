use core::panic;

use crate::model::{Expression, Node, Statement, Statements};
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

    fn peek(&self) -> Option<&Token> {
        self.tokens.get(self.current)
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

    fn previous_token(&self) -> Option<Token> {
        self.tokens.get(self.current - 1).cloned()
    }

    fn exponent(&mut self) -> Expression {
        let mut expr = self.factor();
        while self.match_token(TokenType::TokCaret) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::BinOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.exponent()),
                    }
                }
                None => panic!("Couldn't get previous_token"),
            };
        }
        expr
    }

    fn modulo(&mut self) -> Expression {
        let mut expr = self.exponent();
        while self.match_token(TokenType::TokMod) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::BinOp {
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

    fn term(&mut self) -> Expression {
        let mut expr = self.modulo();
        while self.match_token(TokenType::TokStar) || self.match_token(TokenType::TokSlash) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::BinOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.modulo()),
                    }
                }
                None => panic!("Couldn't get previous_token"),
            };
        }
        expr
    }

    fn expr(&mut self) -> Expression {
        let mut expr = self.term();
        while self.match_token(TokenType::TokPlus) || self.match_token(TokenType::TokMinus) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::BinOp {
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
        if self.match_token(TokenType::TokInteger) {
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
            let expr = self.logical_or();
            if self.match_token(TokenType::TokRparen) {
                return Expression::Grouping {
                    value: Box::new(expr),
                };
            }
            panic!("Unable to find closing parentheses");
        }
        if self.match_token(TokenType::TokTrue) {
            match self.previous_token() {
                Some(_) => return Expression::Bool { value: true },
                _ => panic!("Unable to parse primary token"),
            }
        }
        if self.match_token(TokenType::TokFalse) {
            match self.previous_token() {
                Some(_) => return Expression::Bool { value: false },
                _ => panic!("Unable to parse primary token"),
            }
        }
        if self.match_token(TokenType::TokString) {
            match self.previous_token() {
                Some(token) => {
                    let len = token.lexeme.len();
                    return Expression::Str {
                        value: token.lexeme[1..len - 1].to_string(),
                    };
                }
                _ => panic!("Unable to parse primary token"),
            }
        }
        Expression::Identifier {
            name: self.expect(TokenType::TokIdentifier).lexeme,
        }
    }

    fn unary(&mut self) -> Expression {
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
        self.unary()
    }

    fn comparison(&mut self) -> Expression {
        let mut expr = self.expr();
        while self.match_token(TokenType::TokGe)
            || self.match_token(TokenType::TokLe)
            || self.match_token(TokenType::TokGt)
            || self.match_token(TokenType::TokLt)
        {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::BinOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.expr()),
                    }
                }
                _ => panic!("No previous_token"),
            }
        }
        expr
    }

    fn equality(&mut self) -> Expression {
        let mut expr = self.comparison();
        while self.match_token(TokenType::TokEq) || self.match_token(TokenType::TokNe) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::BinOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.comparison()),
                    }
                }
                None => panic!("No previous_token"),
            };
        }
        expr
    }

    fn logical_and(&mut self) -> Expression {
        let mut expr = self.equality();
        while self.match_token(TokenType::TokAnd) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::LogicalOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.equality()),
                    }
                }
                None => panic!("No previous_token"),
            };
        }
        expr
    }

    fn logical_or(&mut self) -> Expression {
        let mut expr = self.logical_and();
        while self.match_token(TokenType::TokOr) {
            match self.previous_token() {
                Some(op) => {
                    expr = Expression::LogicalOp {
                        op,
                        left: Box::new(expr),
                        right: Box::new(self.logical_and()),
                    }
                }
                None => panic!("No previous_token"),
            };
        }
        expr
    }

    fn if_stmt(&mut self) -> Statement {
        self.expect(TokenType::TokIf);
        let test = self.logical_or();
        self.expect(TokenType::TokThen);
        let then_stmts = self.stmts();
        let mut else_stmts: Statements = Vec::new();
        if self.is_next(TokenType::TokElse) {
            self.advance();
            else_stmts = self.stmts();
        }
        self.expect(TokenType::TokEnd);
        Statement::IfStatement {
            test,
            then_stmts,
            else_stmts,
        }
    }

    fn while_stmt(&mut self) -> Statement {
        self.expect(TokenType::TokWhile);
        let test = self.logical_or();
        self.expect(TokenType::TokThen);
        let stmts = self.stmts();
        self.expect(TokenType::TokEnd);
        return Statement::While { test, stmts };
    }

    fn println_stmt(&mut self) -> Statement {
        if self.match_token(TokenType::TokPrintln) {
            return Statement::PrintlnStatement {
                value: self.logical_or(),
            };
        }
        panic!("Wrong token type")
    }

    fn print_stmt(&mut self) -> Statement {
        if self.match_token(TokenType::TokPrint) {
            return Statement::PrintStatement {
                value: self.logical_or(),
            };
        }
        panic!("Wrong token type")
    }

    fn stmt(&mut self) -> Statement {
        let token = self.peek();
        match token {
            Some(op) => match op.token_type {
                TokenType::TokPrint => self.print_stmt(),
                TokenType::TokPrintln => self.println_stmt(),
                TokenType::TokWhile => self.while_stmt(),
                TokenType::TokIf => self.if_stmt(),
                _ => {
                    let left = self.expr();
                    if self.match_token(TokenType::TokAssign) {
                        return Statement::Assignment {
                            left,
                            right: self.expr(),
                        };
                    }
                    panic!("Not handling function calls yet");
                }
            },
            None => panic!("Failed to get token"),
        }
    }

    fn stmts(&mut self) -> Statements {
        let mut stmts_vec: Statements = Vec::new();
        while self.current < self.tokens.len()
            && !self.is_next(TokenType::TokEnd)
            && !self.is_next(TokenType::TokElse)
        {
            stmts_vec.push(self.stmt());
        }
        stmts_vec
    }

    pub fn parse(&mut self) -> Node {
        Node::Stmts(self.stmts())
    }
}
