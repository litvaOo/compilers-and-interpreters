use std::{env, fs};

use crate::lexer::Lexer;

pub mod interpreter;
pub mod lexer;
pub mod model;
pub mod parser;
pub mod state;
pub mod token;

fn main() {
    let args: Vec<String> = env::args().collect();
    let contents = fs::read_to_string(&args[1]).expect("Read failed");
    let mut lexer = lexer::lexer_init(contents);
    lexer.tokenize();
    let mut parser = parser::Parser::new(lexer.tokens);
    let parsed_tokens = parser.parse();
    let interpreter = interpreter::Interpreter::new();
    println!("{}", interpreter.interpret_ast(parsed_tokens));
}
