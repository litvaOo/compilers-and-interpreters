import sys
from tokens import Token
from lexer import Lexer


def main():
    if len(sys.argv) != 2:
        raise SystemExit("No file passed")
    filename = sys.argv[1]
    print(filename)

    with open(filename) as file:
        source = file.read()

    lexer = Lexer(source)
    lexer.tokenize()
    for token in lexer.tokens:
        print(token)


if __name__ == "__main__":
    main()
