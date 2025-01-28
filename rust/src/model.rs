use std::fmt::Display;

use crate::token::Token;

#[derive(Debug, Clone)]
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
    Identifier {
        name: String,
    },
}

pub type Statements = Vec<Statement>;

#[derive(Debug, Clone)]
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
    Assignment {
        left: Expression,
        right: Expression,
    },
    While {
        test: Expression,
        stmts: Statements,
    },
    For {
        identifier: Expression,
        start: Expression,
        end: Expression,
        step: Expression,
        stmts: Statements,
    },
}

#[derive(Debug, Clone)]
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
            Statement::Assignment { left, right } => write!(f, "{}, {}", left, right),
            Statement::While { test, stmts } => write!(f, "{}, {:?}", test, stmts),
            Statement::For {
                identifier,
                start,
                end,
                step,
                stmts,
            } => write!(
                f,
                "{}, {}, {}, {}, {:?}",
                identifier, start, end, step, stmts
            ),
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
            Expression::Identifier { name } => {
                write!(f, "({})", name)
            }
        }
    }
}
