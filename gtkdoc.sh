#!/bin/bash -x
# https://wiki.gnome.org/DocumentationProject/GtkDoc

MYDIR=${PWD}

make Debug

rm -fr gtkdoc
mkdir gtkdoc
cd gtkdoc

gtkdoc-scan \
	--module=tn3270 \
	--source-dir ${MYDIR}/src/include

LD_LIBRARY_PATH=${MYDIR}/.bin/Debug \
	gtkdoc-scangobj \
	--cflags "$(pkg-config --cflags gtk+-3.0 gmodule-2.0 glib-2.0)" \
	--ldflags "$(pkg-config --libs gtk+-3.0 gmodule-2.0 glib-2.0) -L../.bin/Debug -lv3270 -l3270" \
	--module tn3270

gtkdoc-mkdb \
	--module tn3270 \
	--output-format xml \
	--source-dir ${MYDIR}/src

