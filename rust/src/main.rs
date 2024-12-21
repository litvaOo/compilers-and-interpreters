use std::{env, fs};

use crate::lexer::Lexer;

pub mod interpreter;
pub mod lexer;
pub mod model;
pub mod parser;
pub mod token;

fn main() {
    let args: Vec<String> = env::args().collect();
    let contents = fs::read_to_string(&args[1]).expect("Read failed");
    // println!("{contents}");
    let mut lexer = lexer::lexer_init(contents);
    lexer.tokenize();
    for token in &lexer.tokens {
        println!("{}", token);
    }
    let mut parser = parser::Parser::new(lexer.tokens);
    println!("{}", parser.parse())
}
