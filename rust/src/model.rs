use std::fmt::Display;

use crate::token::Token;

#[derive(Debug)]
pub enum Expression {
    Integer {
        value: i64,
    },
    Float {
        value: f64,
    },
    UnaryOp {
        op: Token,
        exp: Box<Expression>,
    },
    BinOp {
        op: Token,
        left: Box<Expression>,
        right: Box<Expression>,
    },
    Grouping {
        value: Box<Expression>,
    },
}

impl Display for Expression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expression::Integer { value } => write!(f, "{}", value),
            Expression::Float { value } => write!(f, "{}", value),
            Expression::UnaryOp { op, exp } => {
                write!(f, "({} {})", op.lexeme, exp)
            }
            Expression::BinOp { op, left, right } => {
                write!(f, "({} {} {})", op.lexeme, left, right)
            }
            Expression::Grouping { value } => {
                write!(f, "({})", value)
            }
        }
    }
}
