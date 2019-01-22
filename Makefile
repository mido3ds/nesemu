# auto-generated makefile

.PHONY: all cmake build run

all: cmake build run

cmake:
	cmake -S. -B"C:/Users/Mahmoud/Practice/nesemu/build" || echo ">>>> cmake initialization failed! <<<<<"

build:
	cmake --build "C:/Users/Mahmoud/Practice/nesemu/build" || echo ">>>> build failed! <<<<<"

run:
	"C:/Users/Mahmoud/Practice/nesemu/build/NesEmu/Debug/NesEmu.exe"

clean:
	cmake --build "C:/Users/Mahmoud/Practice/nesemu/build" --target clean || echo ">>>> clean failed! <<<<<"

cleanAll:
	      del Makefile && rd /s /q build && rd /s /q CMakeFiles || rm -rf build CMakeFiles Makefile
