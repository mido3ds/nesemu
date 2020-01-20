# nesemu [WIP]
NES emulator in modern c++

## Dependencies:
- sdl2
- yaml-cpp

## After clone:
- install libjack-dev, if you are on ubuntu/debian run `$ sudo apt install -y libjack-dev`
- install latest version of cmake and conan
- `$ ./init.sh`

## Build:
`$ make build`

## Run:
`$ make run [rom=/path/to/rom.ines] [config=/path/to/config.yaml]`
