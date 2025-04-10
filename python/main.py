import sys
from lexer import Lexer
from parser import Parser

# from interpreter import Interpreter
from compiler import Compiler


def main():
    if len(sys.argv) != 2:
        raise SystemExit("No file passed")
    filename = sys.argv[1]

    with open(filename) as file:
        source = file.read()

    lexer = Lexer(source)
    lexer.tokenize()
    # for token in lexer.tokens:
    #     print(token)
    parser = Parser(lexer.tokens)
    ast = parser.parse()
    # print(ast)

    compiler = Compiler()
    compiler.compile_code(ast)
    print(compiler)
    # interpreter = Interpreter()
    # interpreter.interpret_ast(ast)


if __name__ == "__main__":
    main()
