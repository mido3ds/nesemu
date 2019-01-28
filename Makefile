# auto-generated makefile

.PHONY: all cmake build run

all: cmake build run

cmake:
	cmake -S. -B"C:/Users/Mahmoud/practice/nesemu/build" || echo ">>>> cmake initialization failed! <<<<<"

build:
	cmake --build "C:/Users/Mahmoud/practice/nesemu/build" || echo ">>>> build failed! <<<<<"

run:
	"C:/Users/Mahmoud/practice/nesemu/build/NesEmu/Debug/NesEmu.exe" mario.nes

clean:
	cmake --build "C:/Users/Mahmoud/practice/nesemu/build" --target clean || echo ">>>> clean failed! <<<<<"

cleanAll:
	      del Makefile && rd /s /q build && rd /s /q CMakeFiles || rm -rf build CMakeFiles Makefile
