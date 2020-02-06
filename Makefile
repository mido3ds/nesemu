rom=donkey-kong.nes

all: build compile run

install-libraries:
	@set -e
	apt update 
	apt install -y libsdl2-dev libsdl2-ttf-dev

build:
	cmake . -Bbuild

compile:
	cmake --build build -j 10

run:
	./build/NesEmu/NesEmu $(rom)

clean:
	cmake --build build --target clean 

cleanAll:
	rm -rf build CMakeFiles 
