all: game.c
	clang --target=wasm32 -nostdlib -c game.c
	wasm-ld --no-entry --export-all --allow-undefined \
		-o ./public/game.wasm game.o
	rm game.o