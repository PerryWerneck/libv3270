## 3270 Virtual Terminal for GTK.

Created originally as part of [PW3270](../../../pw3270) application.

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![CodeQL](https://github.com/PerryWerneck/libv3270/workflows/CodeQL/badge.svg?branch=master)
[![build result](https://build.opensuse.org/projects/home:PerryWerneck:pw3270/packages/libv3270/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:PerryWerneck:pw3270/libv3270)
[![Publish](https://github.com/PerryWerneck/libv3270/actions/workflows/publish.yml/badge.svg)](https://github.com/PerryWerneck/libv3270/actions/workflows/publish.yml)

## Instalation

### Linux

[<img src="https://raw.githubusercontent.com/PerryWerneck/pw3270/master/branding/obs-badge-en.svg" alt="Download from open build service" height="80px">]([https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270](https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=libv3270))

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

## Building for macOS

### Using homebrew

1. Build and install [lib3270](../../../lib3270)

2. Install additional dependencies

	```shell
	brew update
	brew install xz meson ninja curl gettext openssl pkgconfig gtk+3
	brew upgrade
	```

3. Get libv3270 sources from git

	```shell
	git clone https://github.com/PerryWerneck/lib3270.git ./libv3270
	cd libv3270
	```

4. Configure, build and install

	```shell
	meson setup --prefix=$(brew --prefix)/Cellar/libv3270/$(grep 'version:' meson.build | cut -d: -f2 | cut -d\' -f2) --reconfigure --wipe .build
	meson compile -C .build
	meson install -C .build
	brew link libv3270
	```

To uninstall

	```shell
	brew unlink libv3270
	rm -fr "$(brew --cellar)/libv3270"
	```

### Using jhbuild

1. Install jhbuild and GTK-OSX

	https://wiki.gnome.org/Projects/GTK/OSX/Building
	
2. build

	```shell
	jhbuild --moduleset=https://raw.githubusercontent.com/PerryWerneck/libv3270/master/mac/libv3270.modules build libv3270
	```


