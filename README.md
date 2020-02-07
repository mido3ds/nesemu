# nesemu [WIP]
NES emulator in modern c++

## Dependencies:
- cmake `+3.12.1`
- sdl2 `+2.0.8`
- sdl2_ttf `+2.0.14`
- Catch2 `+2.11.1`

## After Clone:
1. install dependencies. If you have apt, run: `$ sudo ./scripts/install-libraries`
1. run cmake in `Debug` mode: `$ BUILD_TYPE=Debug ./scripts/cmake`
    
    or in `Release` mode: `$ BUILD_TYPE=Release ./scripts/cmake`

## Build:
`$ ./scripts/build [target]`

## Run:
`$ ./scripts/run /path/to/rom.ines`

## Run All Tests:
`$ ./scripts/test`

## Run Speicific Test:
`$ ./scripts/test <name of test without file suffix>`
