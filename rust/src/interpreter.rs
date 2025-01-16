use crate::model::{Expression, Node, Statement};
use crate::token::TokenType;
use core::panic;
use std::fmt;
use std::fmt::{Display, Formatter};
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
    Null,
}

impl Display for ResultType {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        match self {
            ResultType::Number(value) => write!(f, "{}", value),
            ResultType::Bool(value) => write!(f, "{}", value),
            ResultType::Str(value) => write!(f, "{}", value),
            ResultType::Null => write!(f, ""),
        }
    }
}

impl Interpreter {
    pub fn new() -> Interpreter {
        Interpreter {}
    }

    pub fn interpret(self: &Interpreter, node: Node) -> ResultType {
        match node {
            Node::Stmts(stmts) => {
                for stmt in stmts {
                    self.interpret(Node::Stmt(stmt));
                }
                ResultType::Null
            }
            Node::Stmt(stmt) => match stmt {
                Statement::PrintlnStatement { value } => {
                    print!("{}", self.interpret(Node::Expr(value)));
                    ResultType::Null
                }
                Statement::PrintStatement { value } => {
                    println!("{}", self.interpret(Node::Expr(value)));
                    ResultType::Null
                }
                Statement::IfStatement {
                    test,
                    then_stmts,
                    else_stmts,
                } => {
                    let express = self.interpret(Node::Expr(test));
                    match express {
                        ResultType::Number(num) => {
                            if num != 0.0 {
                                return self.interpret(Node::Stmts(then_stmts));
                            }
                            self.interpret(Node::Stmts(else_stmts))
                        }
                        ResultType::Bool(val) => {
                            if val {
                                return self.interpret(Node::Stmts(then_stmts));
                            }
                            self.interpret(Node::Stmts(else_stmts))
                        }
                        ResultType::Str(string) => {
                            if !string.is_empty() {
                                return self.interpret(Node::Stmts(then_stmts));
                            }
                            self.interpret(Node::Stmts(else_stmts))
                        }
                        ResultType::Null => {
                            panic!("Interpreted Statement in an if, that's not good")
                        }
                    }
                }
            },

            Node::Expr(expr) => match expr {
                Expression::Integer { value } => ResultType::Number(value as f64),
                Expression::Float { value } => ResultType::Number(value),
                Expression::Bool { value } => ResultType::Bool(value),
                Expression::Str { value } => ResultType::Str(value),
                Expression::Grouping { value } => self.interpret(Node::Expr(*value)),
                Expression::LogicalOp { op, left, right } => {
                    let lres = self.interpret(Node::Expr(*left));
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
                    self.interpret(Node::Expr(*right))
                }
                Expression::BinOp { op, left, right } => {
                    let lres = self.interpret(Node::Expr(*left));
                    let rres = self.interpret(Node::Expr(*right));
                    if let ResultType::Number(lvalue) = lres {
                        if let ResultType::Number(rvalue) = rres {
                            match op.token_type {
                                TokenType::TokPlus => return ResultType::Number(lvalue + rvalue),
                                TokenType::TokMinus => return ResultType::Number(lvalue - rvalue),
                                TokenType::TokStar => return ResultType::Number(lvalue * rvalue),
                                TokenType::TokSlash => return ResultType::Number(lvalue / rvalue),
                                TokenType::TokMod => return ResultType::Number(lvalue % rvalue),
                                TokenType::TokCaret => {
                                    return ResultType::Number(lvalue.powf(rvalue))
                                }
                                TokenType::TokEq => return ResultType::Bool(lvalue == rvalue),
                                TokenType::TokLe => return ResultType::Bool(lvalue <= rvalue),
                                TokenType::TokLt => return ResultType::Bool(lvalue < rvalue),
                                TokenType::TokGe => return ResultType::Bool(lvalue >= rvalue),
                                TokenType::TokGt => return ResultType::Bool(lvalue > rvalue),
                                TokenType::TokNe => return ResultType::Bool(lvalue != rvalue),
                                _ => panic!("Incompatible operation"),
                            }
                        }
                        if let ResultType::Str(rvalue) = rres {
                            match op.token_type {
                                TokenType::TokPlus => {
                                    return ResultType::Str(
                                        lvalue.to_string() + &(rvalue.to_owned()),
                                    )
                                }
                                TokenType::TokStar => {
                                    if lvalue.is_sign_positive() && lvalue == lvalue.ceil() {
                                        return ResultType::Str(
                                            rvalue.repeat(lvalue.ceil() as usize),
                                        );
                                    }
                                    panic!("Incompatible operation");
                                }
                                _ => panic!("Incompatible operation"),
                            }
                        }
                        panic!("Incompatible operation");
                    }
                    if let ResultType::Bool(lvalue) = lres {
                        if let ResultType::Bool(rvalue) = rres {
                            match op.token_type {
                                TokenType::TokEq => return ResultType::Bool(lvalue == rvalue),
                                TokenType::TokNe => return ResultType::Bool(lvalue != rvalue),
                                _ => panic!("Incompatible operation"),
                            }
                        }
                        if let ResultType::Number(rvalue) = rres {
                            match op.token_type {
                                TokenType::TokEq => {
                                    return ResultType::Bool(lvalue as i64 as f64 == rvalue)
                                }
                                TokenType::TokNe => {
                                    return ResultType::Bool(lvalue as i64 as f64 != rvalue)
                                }
                                TokenType::TokPlus => {
                                    return ResultType::Number(lvalue as i64 as f64 + rvalue)
                                }
                                TokenType::TokMinus => {
                                    return ResultType::Number(lvalue as i64 as f64 - rvalue)
                                }
                                TokenType::TokStar => {
                                    return ResultType::Number(lvalue as i64 as f64 * rvalue)
                                }
                                TokenType::TokSlash => {
                                    return ResultType::Number(lvalue as i64 as f64 / rvalue)
                                }
                                _ => panic!("Incompatible operation"),
                            }
                        }
                        panic!("Incompatible operation");
                    }
                    if let ResultType::Bool(rvalue) = rres {
                        if let ResultType::Number(lvalue) = lres {
                            match op.token_type {
                                TokenType::TokEq => {
                                    return ResultType::Bool(lvalue == rvalue.into())
                                }
                                TokenType::TokNe => {
                                    return ResultType::Bool(lvalue != rvalue.into())
                                }
                                TokenType::TokPlus => {
                                    return ResultType::Number(lvalue + rvalue as i64 as f64)
                                }
                                TokenType::TokMinus => {
                                    return ResultType::Number(lvalue - rvalue as i64 as f64)
                                }
                                TokenType::TokStar => {
                                    return ResultType::Number(lvalue * rvalue as i64 as f64)
                                }
                                TokenType::TokSlash => {
                                    return ResultType::Number(lvalue / rvalue as i64 as f64)
                                }
                                _ => panic!("Incompatible operation"),
                            }
                        }
                    }
                    if let ResultType::Str(lvalue) = lres {
                        if let ResultType::Str(rvalue) = rres {
                            match op.token_type {
                                TokenType::TokEq => return ResultType::Bool(lvalue == rvalue),
                                TokenType::TokNe => return ResultType::Bool(lvalue != rvalue),
                                TokenType::TokPlus => return ResultType::Str(lvalue + &rvalue),
                                _ => panic!("Incompatible operation"),
                            }
                        }
                        if let ResultType::Number(rvalue) = rres {
                            match op.token_type {
                                TokenType::TokPlus => {
                                    return ResultType::Str(lvalue + &(rvalue.to_string()))
                                }
                                TokenType::TokStar => {
                                    if rvalue.is_sign_positive() && rvalue == rvalue.ceil() {
                                        return ResultType::Str(
                                            lvalue.repeat(rvalue.ceil() as usize),
                                        );
                                    }
                                }
                                _ => panic!("Incompatible operation"),
                            }
                        }
                        panic!("Incompatible operation");
                    }
                    panic!("Incompatible operation");
                }
                Expression::UnaryOp { op, exp } => {
                    let rres = self.interpret(Node::Expr(*exp));
                    match op.token_type {
                        TokenType::TokMinus => {
                            if let ResultType::Number(rvalue) = rres {
                                return ResultType::Number(-rvalue);
                            }
                            panic!("Incompatible operation");
                        }
                        TokenType::TokPlus => {
                            if let ResultType::Number(rvalue) = rres {
                                return ResultType::Number(rvalue);
                            }
                            panic!("Incompatible operation");
                        }
                        TokenType::TokNot => {
                            if let ResultType::Bool(rvalue) = rres {
                                return ResultType::Bool(!rvalue);
                            }
                            panic!("Incompatible operation");
                        }
                        _ => panic!("Incompatible operation"),
                    }
                }
            },
        }
    }
}
