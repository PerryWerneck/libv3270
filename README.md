## 3270 Virtual Terminal for GTK.

Created originally as part of PW3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![CodeQL](https://github.com/PerryWerneck/libv3270/workflows/CodeQL/badge.svg?branch=master)
![Downloads](https://img.shields.io/github/downloads/PerryWerneck/libv3270/total.svg)

## Instalation

### Linux

For the supported distributions get the install repositories and instructions from https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=libv3270

### Windows

 * TODO

## Building for Linux

### Requirements

 * [GTK3](https://www.gtk.org/)
 * [lib3270](../../../lib3270)

### Building

 * TODO


## Building for windows

### Cross-compiling on SuSE Linux (Native or WSL)

1. Add the MinGW Repositories

	```
	sudo zypper ar obs://windows:mingw:win32 mingw32
	sudo zypper ar obs://windows:mingw:win64 mingw64
	sudo zypper ar obs://home:PerryWerneck:pw3270 pw3270
	sudo zypper ref
	```

2. Get libv3270 sources from git

	```
	git clone https://github.com/PerryWerneck/libv3270.git ./v3270
	```

3. Install 64 bits cross compilers

	```
	./v3270/win/install-cross.sh --64
	```

4. Configure 64 bits build environment

	```
	./v3270/win/win-configure.sh --64
	```

5. Build

	```
	cd v3270
	make clean
	make all
	```

### Windows native with MSYS2

1. Build and install [lib3270](../../../lib3270)

2. Install the required libraries

	```
	pacman -S mingw-w64-x86_64-gtk3
	```

2. Get libv3270 sources from git

	```
	git clone https://github.com/PerryWerneck/libv3270.git ./libv3270
	```

4. Build library using the mingw shell

	```
	cd libv3270
	./autogen.sh
	make all
	```

## Building for macOS (using homebrew)

1. Build and install [lib3270](../../../lib3270)

2. Install additional dependencies

	```shell
	brew install gtk+3
	```

3. Configure, build and install

	```shell
	./autogen.sh --prefix="$(brew --cellar)/libv3270/5.3"
	make all && make install
	brew link libv3270
	```

### Uninstalling

1. To uninstall

	```shell
	brew unlink libv3270
	rm -fr "$(brew --cellar)/libv3270"
	```

