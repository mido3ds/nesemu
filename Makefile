rom=donkey-kong.nes
type=Debug

all: build compile run

install-libraries:
	@set -e
	apt update 
	apt install -y libsdl2-dev libsdl2-ttf-dev

build:
	cmake -D BUILD_TYPE=$(type) . -Bbuild

compile:
	cmake --build build -j 10

run:
	./build/nesemu $(rom)

clean:
	cmake --build build --target clean 

cleanAll:
	rm -rf build* CMakeFiles 
