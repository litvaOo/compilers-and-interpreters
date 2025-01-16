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
    Bool {
        value: bool,
    },
    Str {
        value: String,
    },
    UnaryOp {
        op: Token,
        exp: Box<Expression>,
    },
    LogicalOp {
        op: Token,
        left: Box<Expression>,
        right: Box<Expression>,
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

pub type Statements = Vec<Statement>;

#[derive(Debug)]
pub enum Statement {
    PrintStatement {
        value: Expression,
    },
    PrintlnStatement {
        value: Expression,
    },
    IfStatement {
        test: Expression,
        then_stmts: Statements,
        else_stmts: Statements,
    },
}

#[derive(Debug)]
pub enum Node {
    Stmts(Statements),
    Stmt(Statement),
    Expr(Expression),
}

impl Display for Statement {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Statement::PrintlnStatement { value } => write!(f, "{}", value),
            Statement::PrintStatement { value } => write!(f, "{}", value),
            Statement::IfStatement {
                test,
                then_stmts,
                else_stmts,
            } => write!(f, "{}, then {:?}, else {:?}", test, then_stmts, else_stmts),
        }
    }
}

#[derive(Debug)]
pub enum VariableType {
    TypeBool,
    TypeString,
    TypeNumber,
}

impl Display for Expression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expression::Integer { value } => write!(f, "{}", value),
            Expression::Float { value } => write!(f, "{}", value),
            Expression::Str { value } => write!(f, "{}", value),
            Expression::Bool { value } => write!(f, "{}", value),
            Expression::UnaryOp { op, exp } => {
                write!(f, "({} {})", op.lexeme, exp)
            }
            Expression::BinOp { op, left, right } => {
                write!(f, "({} {} {})", op.lexeme, left, right)
            }
            Expression::LogicalOp { op, left, right } => {
                write!(f, "({} {} {})", op.lexeme, left, right)
            }
            Expression::Grouping { value } => {
                write!(f, "({})", value)
            }
        }
    }
}
