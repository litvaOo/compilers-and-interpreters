use crate::model::{Expression, Node, Statement};
use crate::state::State;
use crate::token::TokenType;
use core::panic;
use std::cell::RefCell;
use std::fmt::{self};
use std::fmt::{Display, Formatter};
use std::rc::Rc;

#[derive(Debug, Clone)]
pub enum ResultType {
    Number(f64),
    Bool(bool),
    Str(String),
    Null,
    Return(Box<ResultType>),
}

impl Display for ResultType {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        match self {
            ResultType::Number(value) => write!(f, "{}", value),
            ResultType::Bool(value) => write!(f, "{}", value),
            ResultType::Str(value) => write!(f, "{}", unescape_string(value)),
            ResultType::Return(value) => write!(f, "{}", value),
            ResultType::Null => write!(f, ""),
        }
    }
}

fn unescape_string(s: &str) -> String {
    let mut chars = s.chars().peekable();
    let mut result = String::new();

    while let Some(c) = chars.next() {
        if c == '\\' {
            match chars.next() {
                Some('n') => result.push('\n'),
                Some('r') => result.push('\r'),
                Some('t') => result.push('\t'),
                Some('\\') => result.push('\\'),
                Some('\"') => result.push('\"'),
                Some(x) => result.push(x),
                None => (),
            }
        } else {
            result.push(c);
        }
    }

    result
}

pub fn interpret_ast(node: Node) -> ResultType {
    let state = Rc::new(RefCell::new(State::new(None)));
    interpret(node, &state)
}

pub fn interpret(node: Node, state: &Rc<RefCell<State>>) -> ResultType {
    match node {
        Node::Stmts(stmts) => {
            for stmt in stmts {
                let res = interpret(Node::Stmt(stmt), state);
                if let ResultType::Return(_returned) = res.clone() {
                    return res;
                }
            }
            ResultType::Null
        }
        Node::Stmt(stmt) => match &stmt {
            Statement::PrintlnStatement { value } => {
                println!("{}", interpret(Node::Expr((*value).clone()), state));
                ResultType::Null
            }
            Statement::PrintStatement { value } => {
                print!("{}", interpret(Node::Expr((*value).clone()), state));
                ResultType::Null
            }
            Statement::LocalAssignment { left, right } => {
                let rres = interpret(Node::Expr((*right).clone()), state);
                match left {
                    Expression::Identifier { name } => {
                        state.borrow_mut().set_local_item((*name).clone(), rres)
                    }
                    _ => panic!("Assigning not to identifier"),
                }
                ResultType::Null
            }
            Statement::Return { val } => {
                ResultType::Return(Box::new(interpret(Node::Expr((*val).clone()), state)))
            }
            Statement::FunctionDeclaration {
                name,
                params: _,
                stmts: _,
            } => {
                state
                    .borrow_mut()
                    .set_function((*name).clone(), stmt.clone());
                ResultType::Null
            }
            Statement::FunctionCall { expr } => interpret(Node::Expr((*expr).clone()), state),
            Statement::Parameter { name: _ } => ResultType::Null,
            Statement::For {
                identifier,
                start,
                end,
                step,
                stmts,
            } => {
                let mut new_env = State::get_child_env(state);
                if let Expression::Identifier { name } = identifier {
                    let start_res = interpret(
                        Node::Expr((*start).clone()),
                        &Rc::new(RefCell::new(new_env.clone())),
                    );
                    if let ResultType::Number(start_val) = start_res {
                        new_env.set_item(name.clone(), start_res);
                        let end_res = interpret(
                            Node::Expr((*end).clone()),
                            &Rc::new(RefCell::new(new_env.clone())),
                        );
                        let step_res = interpret(
                            Node::Expr((*step).clone()),
                            &Rc::new(RefCell::new(new_env.clone())),
                        );
                        if let ResultType::Number(end_val) = end_res {
                            if let ResultType::Number(step_val) = step_res {
                                loop {
                                    if let ResultType::Number(current_value) =
                                        new_env.get_item(name).unwrap()
                                    {
                                        if (start_val >= end_val && current_value <= end_val)
                                            || (start_val <= end_val && current_value >= end_val)
                                        {
                                            break;
                                        }
                                        let res = interpret(
                                            Node::Stmts(stmts.clone()),
                                            &Rc::new(RefCell::new(new_env.clone())),
                                        );
                                        if let ResultType::Return(_returned) = res.clone() {
                                            return res;
                                        }
                                        new_env.set_item(
                                            name.clone(),
                                            ResultType::Number(current_value + step_val),
                                        );
                                    }
                                }
                            }
                        }
                    }
                }
                ResultType::Null
            }
            Statement::While { test, stmts } => {
                let new_env = State::get_child_env(state);
                loop {
                    let test_res = interpret(
                        Node::Expr(test.clone()),
                        &Rc::new(RefCell::new(new_env.clone())),
                    );
                    match &test_res {
                        ResultType::Bool(val) => {
                            if !val {
                                break;
                            };
                        }
                        ResultType::Str(val) => {
                            if val.is_empty() {
                                break;
                            }
                        }
                        ResultType::Number(val) => {
                            if *val == 0.0 {
                                break;
                            }
                        }
                        ResultType::Return(_val) => return test_res,
                        ResultType::Null => panic!("Shouldn't have null expression in val"),
                    }
                    let res = interpret(
                        Node::Stmts(stmts.clone()),
                        &Rc::new(RefCell::new(new_env.clone())),
                    );
                    if let ResultType::Return(_returned) = res.clone() {
                        return res;
                    }
                }
                ResultType::Null
            }
            Statement::IfStatement {
                test,
                then_stmts,
                else_stmts,
            } => {
                let express = interpret(Node::Expr((*test).clone()), state);
                let new_env = State::get_child_env(state);
                match &express {
                    ResultType::Number(num) => {
                        if *num != 0.0 {
                            return interpret(
                                Node::Stmts((*then_stmts).clone()),
                                &Rc::new(RefCell::new(new_env)),
                            );
                        }
                        interpret(
                            Node::Stmts((*else_stmts).clone()),
                            &Rc::new(RefCell::new(new_env)),
                        )
                    }
                    ResultType::Bool(val) => {
                        if *val {
                            let res = interpret(
                                Node::Stmts((*then_stmts).clone()),
                                &Rc::new(RefCell::new(new_env)),
                            );
                            return res;
                        }
                        interpret(
                            Node::Stmts((*else_stmts).clone()),
                            &Rc::new(RefCell::new(new_env)),
                        )
                    }
                    ResultType::Str(string) => {
                        if !string.is_empty() {
                            return interpret(
                                Node::Stmts((*then_stmts).clone()),
                                &Rc::new(RefCell::new(new_env)),
                            );
                        }
                        interpret(
                            Node::Stmts((*else_stmts).clone()),
                            &Rc::new(RefCell::new(new_env)),
                        )
                    }
                    ResultType::Return(_val) => express,
                    ResultType::Null => {
                        panic!("Interpreted Statement in an if, that's not good")
                    }
                }
            }
            Statement::Assignment { left, right } => {
                let rres = interpret(Node::Expr((*right).clone()), state);
                match left {
                    Expression::Identifier { name } => {
                        state.borrow_mut().set_item((*name).clone(), rres)
                    }
                    _ => panic!("Assigning not to identifier"),
                }
                ResultType::Null
            }
        },
        Node::Expr(expr) => match expr {
            Expression::Integer { value } => ResultType::Number(value as f64),
            Expression::Identifier { name } => match state.borrow().get_item(&name) {
                Some(res) => res,
                None => panic!("No Identifier "),
            },
            Expression::FunctionCall { name, args } => {
                let func = state.borrow().get_function(&name).unwrap();
                if let Statement::FunctionDeclaration {
                    name: _,
                    params,
                    stmts,
                } = func
                {
                    if params.len() != args.len() {
                        panic!(
                            "Params count {} is not equal to function args count {}",
                            args.len(),
                            params.len()
                        );
                    }
                    let mut func_state = State::get_child_env(state);

                    for it in params.iter().zip(args.iter()) {
                        let (param, arg) = it;
                        if let Statement::Parameter { name } = param {
                            func_state.set_local_item(
                                name.to_owned(),
                                interpret(Node::Expr(arg.to_owned()), state),
                            );
                        }
                    }
                    let res = interpret(
                        Node::Stmts(stmts),
                        &Rc::new(RefCell::new(func_state.clone())),
                    );
                    if let ResultType::Return(returned) = res {
                        return *returned;
                    }
                    return res;
                }
                panic!("Didn't get a function declaration from funcs table");
            }
            Expression::Float { value } => ResultType::Number(value),
            Expression::Bool { value } => ResultType::Bool(value),
            Expression::Str { value } => ResultType::Str(value),
            Expression::Grouping { value } => interpret(Node::Expr(*value), state),
            Expression::LogicalOp { op, left, right } => {
                let lres = interpret(Node::Expr(*left), state);
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
                interpret(Node::Expr(*right), state)
            }
            Expression::BinOp { op, left, right } => {
                let lres = interpret(Node::Expr(*left), state);
                let rres = interpret(Node::Expr(*right), state);
                if let ResultType::Number(lvalue) = lres {
                    if let ResultType::Number(rvalue) = rres {
                        match op.token_type {
                            TokenType::TokPlus => return ResultType::Number(lvalue + rvalue),
                            TokenType::TokMinus => return ResultType::Number(lvalue - rvalue),
                            TokenType::TokStar => return ResultType::Number(lvalue * rvalue),
                            TokenType::TokSlash => return ResultType::Number(lvalue / rvalue),
                            TokenType::TokMod => {
                                // because in rust it's not actually a modulo
                                // it's a reminder, and we need a modulo
                                // I love low-level languages
                                return ResultType::Number(((lvalue % rvalue) + rvalue) % rvalue);
                            }
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
                    if let ResultType::Str(rvalue) = rres {
                        match op.token_type {
                            TokenType::TokPlus => {
                                return ResultType::Str(lvalue.to_string() + &(rvalue.to_owned()))
                            }
                            TokenType::TokStar => {
                                if lvalue.is_sign_positive() && lvalue == lvalue.ceil() {
                                    return ResultType::Str(rvalue.repeat(lvalue.ceil() as usize));
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
                            TokenType::TokEq => return ResultType::Bool(lvalue == rvalue.into()),
                            TokenType::TokNe => return ResultType::Bool(lvalue != rvalue.into()),
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
                                    return ResultType::Str(lvalue.repeat(rvalue.ceil() as usize));
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
                let rres = interpret(Node::Expr(*exp), state);
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
