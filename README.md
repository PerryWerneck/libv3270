## 3270 Virtual Terminal for GTK.

Created originally as part of [PW3270](../../../pw3270) application.

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CodeQL Advanced](https://github.com/PerryWerneck/libv3270/actions/workflows/codeql.yml/badge.svg)](https://github.com/PerryWerneck/libv3270/actions/workflows/codeql.yml)
[![build result](https://build.opensuse.org/projects/home:PerryWerneck:pw3270/packages/libv3270/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:PerryWerneck:pw3270/libv3270)
[![Publish](https://github.com/PerryWerneck/libv3270/actions/workflows/publish.yml/badge.svg)](https://github.com/PerryWerneck/libv3270/actions/workflows/publish.yml)

## Instalation

### Linux

[<img src="https://raw.githubusercontent.com/PerryWerneck/pw3270/master/branding/obs-badge-en.svg" alt="Download from open build service" height="80px">]([https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270](https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=libv3270))

## Building for Linux

### Requirements

 * [GTK3](https://www.gtk.org/)
 * [lib3270](../../../lib3270)

## Building for windows

### Cross-compiling on SuSE Linux (Native or WSL)

1. Add the MinGW Repositories

	```
	sudo zypper ar obs://windows:mingw:win32 mingw32
	sudo zypper ar obs://windows:mingw:win64 mingw64
	sudo zypper ref
	```

2. Get libv3270 sources from git

	```
	git clone https://github.com/PerryWerneck/libv3270.git ./v3270
	```

3. Install cross compilers

	```shell
	sudo zypper in \
		pkgconfig \
		gettext-devel \
		mingw64-libcurl-devel \
		mingw64-cross-meson \
		mingw64-libopenssl-devel \
		mingw64-cross-gcc-c++ \
		'mingw64(pkg:gtk+-win32-3.0)' \
		'mingw64(lib:iconv)' \ 
		'mingw64(lib:intl)' 
	```

3. Configure and build

	```shell
	meson setup --cross-file /usr/lib/rpm/macros.d/meson-mingw64-cross-file.txt .build
	meson compile -C .build
	```

### Windows native with MSYS2

1. Install and update MSYS2 

	* Download and install [msys2](https://www.msys2.org/)
	* Update msys:
	
	```shell
	pacman -Syu
	```
	Afther this close and reopen mingw shell.

2. Update system path

	* Add c:\msys64\usr\bin and c:\msys64\mingw64\bin to system path

3. Install devel packages using pacman on mingw shell

	```shell
	pacman -S \
		dos2unix \
		mingw-w64-x86_64-gcc \
		mingw-w64-x86_64-meson \
		mingw-w64-x86_64-iconv \
		pkgconf \
		mingw-w64-x86_64-gettext \
		gettext-devel \
		mingw-w64-x86_64-openssl \
		mingw-w64-x86_64-gtk3
	```

	Afther this close and reopen mingw shell.

4. Get libv3270 sources from git

	```
	git clone https://github.com/PerryWerneck/libv3270.git ./libv3270
	```

5. Build using makepkg

	```shell
	makepkg BUILDDIR=/tmp/pkg -p PKGBUILD.mingw
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


