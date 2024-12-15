import sys
from lexer import Lexer
from parser import Parser
from interpreter import Interpreter


def main():
    if len(sys.argv) != 2:
        raise SystemExit("No file passed")
    filename = sys.argv[1]
    print(filename)

    with open(filename) as file:
        source = file.read()

    print(source)
    lexer = Lexer(source)
    lexer.tokenize()
    for token in lexer.tokens:
        print(token)
    parser = Parser(lexer.tokens)
    ast = parser.parse()
    interpreter = Interpreter()
    print(ast)
    print(interpreter.interpret(ast))


if __name__ == "__main__":
    main()
