import sys

def main():
    if len(sys.argv) != 2:
        raise SystemExit("No file passed")
    filename = sys.argv[1]
    print(filename)

    with open(filename) as file:
        source = file.read()
        print(source)

if __name__ == "__main__":
    main()
