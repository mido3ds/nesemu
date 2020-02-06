# nesemu [WIP]
NES emulator in modern c++

## Dependencies:
- cmake (>= 3.12.1)
- sdl2 (>= 2.0.8)
- sdl2_ttf (>= 2.0.14)

## After clone:
1. install dependencies. If you have apt, run: `$ sudo make install-libraries`
1. build cmake in `Debug` mode: `$ make build type=Debug`
    
    or in `Release` mode: `$ make build type=Release`

## Compile:
`$ make compile`

## Run:
`$ make run rom=/path/to/rom.ines`
