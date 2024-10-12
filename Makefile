CMAKE_MAKE_FLAGS = -S . -B build
CMAKE_BUILD_FLAGS = --build build

all: configure build

configure:
	cmake $(CMAKE_MAKE_FLAGS)

build:
	cmake $(CMAKE_BUILD_FLAGS)

clean:
	rm -rf build

.PHONY: all configure build clean
