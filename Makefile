all: build compile run

build:
	./scripts/cmake

compile: build
	./scripts/build nesemu

run: build
	./scripts/run

testAll: build
	./scripts/test

clean:
	./scripts/clean

cleanAll:
	rm -rf build* CMakeFiles 
