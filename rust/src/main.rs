use std::{env, fs};
fn main() {
    let args: Vec<String> = env::args().collect();
    let contents = fs::read_to_string(&args[1]).expect("Read failed");
    println!("{contents}");
}
