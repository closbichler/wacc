all: build link

clean:
	rm game.wasm game.o

build: game.c
	clang --target=wasm32 -c game.c

link: game.o
	wasm-ld --no-entry --export-all --allow-undefined \
		-o game.wasm game.o

clang: game.c
	lcc --target=wasm32 \
		-O3 -flto -nostdlib \
		-Wl,--no-entry -Wl,--export-all, -Wl,--lto-O3 \
		-o game.wasm game.c