run-c:
	mkdir -p c/target/ && gcc -o c/target/main -lm -g3 c/*.c && c/target/main scripts/main.pinky

run-python:
	mypy python/main.py && python3 python/main.py scripts/main.pinky

run-zig:
	zig run ./zig/src/main.zig -- scripts/main.pinky

run-rust:
	cargo run --manifest-path rust/Cargo.toml -- scripts/main.pinky
