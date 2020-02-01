rom=donkey-kong.nes
config=config.yaml

all: build compile run

.ONESHELL:
build:
	@set -e
	mkdir -p build && cd build

	conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan 2>/dev/null || true
	conan install .. --build=missing
	
	cmake .. -B.

compile:
	cmake --build build

run:
	./build/NesEmu/NesEmu $(rom) $(config)

clean:
	cmake --build build --target clean 

cleanAll:
	rm -rf build CMakeFiles 
