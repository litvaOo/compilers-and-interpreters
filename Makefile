run-c:
	mkdir -p c/target/ && gcc -o c/target/main -lm -g3 c/*.c && c/target/main scripts/main.pinky

run-c-optimized:
	mkdir -p c/target/release && gcc -o c/target/release/main -lm -O3 c/*.c && c/target/release/main scripts/main.pinky

run-python:
	mypy python/main.py && python3 python/main.py scripts/main.pinky

run-zig:
	zig run ./zig/src/main.zig -- scripts/main.pinky

run-rust:
	cargo run --manifest-path rust/Cargo.toml -- scripts/main.pinky

run-rust-optimized:
	cargo build --release --manifest-path rust/Cargo.toml && ./rust/target/release/rust scripts/main.pinky
