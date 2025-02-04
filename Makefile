all: manual-link
	
all-clang: game.c
	clang --target=wasm32 -nostdlib
		Wl,--no-entry Wl,--export-all Wl,--allow-undefined \
		-o ./public/game.wasm game.o

manual-link: game.c
	clang --target=wasm32 -nostdlib -c game.c
	wasm-ld --no-entry --export-all --allow-undefined \
		-o ./public/game.wasm game.o
	rm game.o