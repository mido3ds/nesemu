.PHONY: all cmake build run

rom=donkey-kong.nes
config=config.yaml

all: build run

build:
	cmake --build build

run:
	./build/NesEmu/NesEmu $(rom) $(config)

clean:
	cmake --build build --target clean 

cleanAll:
	rm -rf build CMakeFiles 
