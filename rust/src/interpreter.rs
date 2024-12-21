use core::panic;

use crate::model::{Expression, VariableType};
use crate::token::{Token, TokenType};

pub struct Interpreter {}

impl Default for Interpreter {
    fn default() -> Self {
        Self::new()
    }
}

pub enum ResultType {
    Number(f64),
    Bool(bool),
    Str(String),
}

impl Interpreter {
    pub fn new() -> Interpreter {
        Interpreter {}
    }

    pub fn interpret(self: &Interpreter, node: Expression) -> ResultType {
        match node {
            Expression::Integer { value } => ResultType::Number(value as f64),
            Expression::Float { value } => ResultType::Number(value),
            Expression::Bool { value } => ResultType::Bool(value),
            Expression::Str { value } => ResultType::Str(value),
            Expression::LogicalOp { op, left, right } => {
                let lres = self.interpret(*left);
                if op.token_type == TokenType::TokOr {
                    if let ResultType::Bool(true) = lres {
                        return ResultType::Bool(true);
                    }
                }
                if op.token_type == TokenType::TokAnd {
                    if let ResultType::Bool(false) = lres {
                        return ResultType::Bool(false);
                    }
                }
                self.interpret(*right)
            }
            Expression::BinOp { op, left, right } => {
                let lres = self.interpret(*left);
                let rres = self.interpret(*right);
                if let ResultType::Number(lvalue) = lres {
                    if let ResultType::Number(rvalue) = rres {
                        match op.token_type {
                            TokenType::TokPlus => return ResultType::Number(lvalue + rvalue),
                            TokenType::TokMinus => return ResultType::Number(lvalue - rvalue),
                            TokenType::TokStar => return ResultType::Number(lvalue * rvalue),
                            TokenType::TokSlash => return ResultType::Number(lvalue / rvalue),
                            TokenType::TokMod => return ResultType::Number(lvalue % rvalue),
                            TokenType::TokCaret => return ResultType::Number(lvalue.powf(rvalue)),
                            TokenType::TokEq => return ResultType::Bool(lvalue == rvalue),
                            TokenType::TokLe => return ResultType::Bool(lvalue <= rvalue),
                            TokenType::TokLt => return ResultType::Bool(lvalue < rvalue),
                            TokenType::TokGe => return ResultType::Bool(lvalue >= rvalue),
                            TokenType::TokGt => return ResultType::Bool(lvalue > rvalue),
                            TokenType::TokNe => return ResultType::Bool(lvalue != rvalue),
                            _ => panic!("Incompatible operation"),
                        }
                    }
                } else if let ResultType::Bool(lvalue) = lres {
                    if let ResultType::Bool(rvalue) = rres {
                        match op.token_type {
                            TokenType::TokEq => ResultType::Bool(lvalue == rvalue),
                            TokenType::TokNe => ResultType::Bool(lvalue != rvalue),
                            _ => panic!("Incompatible operation"),
                        }
                    }
                } else if let ResultType::Str(lvalue) = lres {
                    if let ResultType::Str(rvalue) = rres {}
                }
            }
            Expression::UnaryOp { op, exp } => return self.interpret(*exp),
            Expression::Grouping { value } => return self.interpret(*value),
        }
    }
}
