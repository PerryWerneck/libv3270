3270 Virtual Terminal for GTK.
==============================

Created originally as part of PW3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

Installation repositories
=========================

The latest version packaged for many linux distributions can be found in SuSE Build Service (https://build.opensuse.org/project/show/home:PerryWerneck:pw3270)

Requirements
============

 * GTK-3 (https://www.gtk.org/)
 * lib3270 (https://softwarepublico.gov.br/social/pw3270/)


Building for Linux
==================


Cross-compiling for Windows
===========================

Cross-compiling on SuSE Linux (Native or WSL)
---------------------------------------------

1. First add the MinGW Repositories for your SuSE version from:

	* https://build.opensuse.org/project/show/home:PerryWerneck:pw3270 

2. Get libv3270 sources from git

	* git clone https://github.com/PerryWerneck/libv3270.git ./v3270

3. Install cross compilers

	* ./v3270/win/install-cross.sh --32 (for 32 bits)
	* ./v3270/win/install-cross.sh --64 (for 64 bits)
	* ./v3270/win/install-cross.sh --all (for 32 and 64 bits)

3. Configure build

	* ./v3270/win/win-configure.sh --32 (for 32 bits)
	* ./v3270/win/win-configure.sh --64 (for 64 bits)

4. Build

	* cd v3270
	* make clean
	* make all


Compiling for Windows (With MSYS2)
----------------------------------

1. Build and install lib3270 

	* Follow the guide on https://github.com/PerryWerneck/lib3270

2. Get libv3270 sources from git

	* git clone https://github.com/PerryWerneck/libv3270.git ./v3270

4. Build library using the mingw shell

	* cd v3270
	* ./autogen.sh
	* make all


