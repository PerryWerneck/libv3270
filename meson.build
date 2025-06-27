# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2024 Banco do Brasil S. A.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

project(
	'libv3270', 
	['c'],
	version: '5.5.0',
	license: 'GPL-3.0-or-later',
)

project_description = '3270 Virtual Terminal for GTK'

#
# Compiler flags
#
cc = meson.get_compiler('c')

enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
  datecmd = 'date'
else
  datecmd = 'date -u -d@' + time_epoch
endif

compiler_flags = [
  '-ggdb3',
  '-fvisibility=hidden',
  '-pthread',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-DBUILD_DATE='+time_epoch,
]

if enable_debug
  compiler_flags += [
	  '-DDEBUG=1'
  ]
else
  compiler_flags += [
	'-DNDEBUG=1'
  ]
endif

add_project_arguments(cc.get_supported_arguments(compiler_flags), language: 'c')

#
# Versioning
#
pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

#
# i18n
#
subdir('po')

#
# Dependencies
#
lib3270 = dependency('lib3270', required:true)

lib_deps = [
  lib3270,
  dependency('glib-2.0'),
  dependency('gtk+-3.0'),
  dependency('gmodule-2.0'),
  dependency('gdk-3.0'),
]

m_dep = cc.find_library('m', required:false)

extra_deps = [ 
	m_dep 
]

#
# Configuration
#
app_conf = configuration_data()
app_conf.set_quoted('PACKAGE_NAME', meson.project_name())
app_conf.set('PRODUCT_NAME', lib3270.get_variable('product_name'))
app_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
app_conf.set_quoted('PACKAGE_DESCRIPTION', project_description)
app_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
app_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
app_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)

if m_dep.found()
	app_conf.set('HAVE_LIBM', 1)
endif

app_conf.set('HAVE_LIBINTL', 1)
app_conf.set('HAVE_GNUC_VISIBILITY', 1)

rpq_timestamp_value = run_command('sh', '-c', datecmd + ' +%Y%m%d%H%M%S', check : true).stdout().strip()
app_conf.set_quoted('RPQ_TIMESTAMP_VALUE', rpq_timestamp_value)

package_release = run_command('sh', '-c', datecmd + ' +%-y.%-m.%-d', check : true).stdout().strip()
app_conf.set_quoted('PACKAGE_RELEASE', package_release)

if cc.compiles('#include <unistd.h>', name : 'unistd.h')
	app_conf.set('HAVE_UNISTD_H', 1)
endif

#
# Sources
#
includes_dir = include_directories(
    'src/include',
)

lib_src = [
	'src/dialogs/colorscheme.c',
	'src/dialogs/commondialog.c',
	'src/dialogs/dialog.c',
	'src/dialogs/font/chooser.c',
	'src/dialogs/font/model.c',
	'src/dialogs/font/select.c',
	'src/dialogs/load.c',
	'src/dialogs/popups.c',
	'src/dialogs/print/begin.c',
	'src/dialogs/print/convenience.c',
	'src/dialogs/print/draw.c',
	'src/dialogs/print/keyfile.c',
	'src/dialogs/print/print.c',
	'src/dialogs/print/settings.c',
	'src/dialogs/print/settingsdialog.c',
	'src/dialogs/save/convenience.c',
	'src/dialogs/save/save.c',
	'src/dialogs/security.c',
	'src/dialogs/settings/accelerator.c',
	'src/dialogs/settings/clipboard.c',
	'src/dialogs/settings/colors.c',
	'src/dialogs/settings/dialog.c',
	'src/dialogs/settings/host.c',
	'src/dialogs/settings/popup.c',
	'src/dialogs/settings/tools.c',
	'src/dialogs/settings/url.c',
	'src/dialogs/settings/widget.c',
	'src/dialogs/togglebutton.c',
	'src/dialogs/tools.c',
	'src/dialogs/transfer.c',
	'src/filetransfer/activity.c',
	'src/filetransfer/activitylist.c',
	'src/filetransfer/filelist.c',
	'src/filetransfer/get.c',
	'src/filetransfer/load.c',
	'src/filetransfer/misc.c',
	'src/filetransfer/save.c',
	'src/filetransfer/set.c',
	'src/filetransfer/settings.c',
	'src/filetransfer/tables.c',
	'src/filetransfer/transfer.c',
	'src/filetransfer/v3270ft.c',
	'src/filetransfer/v3270ftprogress.c',
	'src/filetransfer/worker.c',
	'src/selection/copy.c',
	'src/selection/datablock.c',
	'src/selection/get.c',
	'src/selection/html.c',
	'src/selection/pixbuf.c',
	'src/selection/selection.c',
	'src/selection/table.c',
	'src/selection/text.c',
	'src/terminal/accessible.c',
	'src/terminal/actions.c',
	'src/terminal/actions/action.c',
	'src/terminal/actions/conditional.c',
	'src/terminal/actions/dialog.c',
	'src/terminal/actions/internal.c',
	'src/terminal/actions/lib3270.c',
	'src/terminal/actions/pakey.c',
	'src/terminal/actions/pfkey.c',
	'src/terminal/actions/print.c',
	'src/terminal/actions/property.c',
	'src/terminal/actions/save.c',
	'src/terminal/actions/scroll.c',
	'src/terminal/actions/simple.c',
	'src/terminal/actions/table.c',
	'src/terminal/actions/toggle.c',
	'src/terminal/actions/zoom.c',
	'src/terminal/blink.c',
	'src/terminal/callbacks.c',
	'src/terminal/charset.c',
	'src/terminal/colors.c',
	'src/terminal/drawing/draw.c',
	'src/terminal/drawing/oia.c',
	'src/terminal/drawing/surface.c',
	'src/terminal/font/actions.c',
	'src/terminal/font/compute.c',
	'src/terminal/font/info.c',
	'src/terminal/font/metrics.c',
	'src/terminal/font/properties.c',
	'src/terminal/font/translate.c',
	'src/terminal/get.c',
	'src/terminal/iocallback.c',
	'src/terminal/keyboard.c',
	'src/terminal/keyboard/accelerator.c',
	'src/terminal/keyboard/init.c',
	'src/terminal/keyboard/keyfile.c',
	'src/terminal/keyboard/map.c',
	'src/terminal/keyfile.c',
	'src/terminal/mouse.c',
	'src/terminal/properties/get.c',
	'src/terminal/properties/init.c',
	'src/terminal/properties/set.c',
	'src/terminal/security.c',
	'src/terminal/toggles.c',
	'src/terminal/widget.c',
	'src/tools/dialog.c',
	'src/tools/entry.c',
	'src/tools/widget.c',
	'src/trace/exec.c',
	'src/trace/property.c',
	'src/trace/text.c',
	'src/trace/trace.c',
	'src/trace/tracewindow.c',
]

if host_machine.system() == 'windows'
  lib_src += [
	'src/dialogs/windows/select.c',
	'src/selection/windows/copy.c',
	'src/selection/windows/paste.c',
	'src/terminal/windows/init.c',
	'src/terminal/windows/iosource.c',
	'src/terminal/windows/registry.c',
  ]
endif

if host_machine.system() == 'linux'
  lib_src += [
	'src/dialogs/linux/select.c',
	'src/terminal/linux/init.c',
	'src/terminal/linux/iosource.c',
	'src/selection/linux/copy.c',
	'src/selection/linux/paste.c',
  ]
endif

if host_machine.system() == 'darwin'
  lib_src += [
	'src/terminal/macos/init.c',
	'src/terminal/macos/iosource.c',
	'src/dialogs/macos/select.c',
	'src/selection/macos/copy.c',
	'src/selection/macos/paste.c',
  ]
endif

#
# Gnome
# https://mesonbuild.com/Gnome-module.html#gnomegenmarshal
#
gnome = import('gnome')

v3270_marshalers = gnome.genmarshal('v3270_marshalers',
  sources: [
	'src/dialogs/settings/marshal',
	'src/terminal/marshal',
  ],
  prefix: 'v3270',
  valist_marshallers: true,
)

v3270ft_marshalers = gnome.genmarshal('v3270ft_marshalers',
  sources: [
	'src/filetransfer/marshal',
  ],
  prefix: 'v3270ft',
  valist_marshallers: true,
)

v3270tr_marshalers = gnome.genmarshal('v3270tr_marshalers',
  sources: [
	'src/trace/marshal',
  ],
  prefix: 'v3270trace',
  valist_marshallers: true,
)

v3270dlg_marshalers = gnome.genmarshal('v3270dlg_marshalers',
  sources: [
	'src/dialogs/settings/marshal',
  ],
  prefix: 'v3270',
  valist_marshallers: true,
)

lib_src += [ 
	v3270_marshalers[0],
	v3270ft_marshalers[0],
	v3270tr_marshalers[0],
	v3270dlg_marshalers[0]
]

#
# SDK
# https://mesonbuild.com/Pkgconfig-module.html
#
pkg = import('pkgconfig')

pkg_variables = [

  'version_major=@0@'.format(pkg_major_version),
  'version_minor=@0@'.format(pkg_minor_version),
  'version_micro=@0@'.format(pkg_micro_version),

  'module_version=@0@.@1@'.format(pkg_major_version,pkg_minor_version),

]

if host_machine.system() == 'windows'

  pkg.generate(
    name: meson.project_name(),
    description: project_description,
    variables: pkg_variables,
    requires: [ 'lib3270' ],
    libraries: [ '-lv3270.dll' ]
  )

#  pkg.generate(
#    name: meson.project_name() + '-static',
#    description: project_description,
#    variables: pkg_variables,
#    requires: [
#		'lib3270-static',
#		'glib-2.0',
#		'gtk+-3.0',
#		'gmodule-2.0',
#		'gdk-3.0',
#    ],
#    libraries: [ 
#      '-lv3270',
#    ]
#  )

else

  pkg.generate(
    name: meson.project_name(),
    description: project_description,
    variables: pkg_variables,
    requires: [ 'lib3270' ],
    libraries: [ '-lv3270' ]
  )

#  pkg.generate(
#    name: meson.project_name() + '-static',
#    description: project_description,
#    variables: pkg_variables,
#    requires: [
#		'lib3270-static',
#		'glib-2.0',
#		'gtk+-3.0',
#		'gmodule-2.0',
#		'gdk-3.0',
#   ],
#    libraries: [ '-l:libv3270.a' ]
#  )

endif

# Check for OSs
#https://mesonbuild.com/Reference-manual_builtin_host_machine.html

if host_machine.system() == 'windows'
  app_conf.set_quoted('LIBEXT', '.dll')
endif

if host_machine.system() == 'linux'
  app_conf.set_quoted('LIBEXT', '.so')  
endif

#
# Targets
#
config_src = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]

if host_machine.system() == 'windows'

  # https://mesonbuild.com/Windows-module.html
  windows = import('windows')
  resources = windows.compile_resources(
    configure_file(
      input : 'src/terminal/windows/resources.rc.in',
      output : 'resources.rc',
      configuration : app_conf
    )
  )

  dynamic = shared_library(
    'v3270', 
    config_src + lib_src + resources,
    install: true,
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps + extra_deps,
	link_args: [
		'-lws2_32',
		'-lwtsapi32',
		'-lcomdlg32',
	],
    include_directories: includes_dir,
  )

elif host_machine.system() == 'darwin'

  dynamic = shared_library(
    'v3270', 
    config_src + lib_src,
    install: true,
    version : libversion,
    soversion : libversion,
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps + extra_deps,
    include_directories: includes_dir
  )

else

  dynamic = shared_library(
    'v3270', 
    config_src + lib_src,
    install: true,
    version : libversion,
    c_args: [ '-ffat-lto-objects' ],
    soversion : libversion,
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps + extra_deps,
    include_directories: includes_dir
  )

  glade_file = configure_file(
	input : 'glade/v3270.xml.in',
	output : 'v3270.xml',
	configuration : app_conf
  )

  install_data(
	glade_file,
	install_tag: 'glade',
	install_dir: get_option('datadir') + '/glade'
  )
 
endif

dynamic_library = declare_dependency(
  link_with : dynamic,
  include_directories : includes_dir,
) 

dynamic_library = declare_dependency(
	link_with: static_library(
	'v3270', 
	config_src + lib_src,
	install: true,
    c_args: [ '-DSTATIC_LIBRARY', '-fno-lto' ],
	dependencies: lib_deps + extra_deps,
	include_directories: includes_dir
	),
  include_directories : includes_dir,
)

executable(
  meson.project_name(),
  config_src + [ 
	'src/testprogram/testprogram.c',
	'src/testprogram/toolbar.c',
  ],
  install: false,
  link_with : [ dynamic ],
  dependencies: lib_deps + extra_deps,
  include_directories: includes_dir
)

install_headers(
	'src/include/v3270.h',
)

install_headers(
	'src/include/v3270/settings/url.h',
	'src/include/v3270/accessible.h',
	'src/include/v3270/actions.h',
	'src/include/v3270/colorscheme.h',
	'src/include/v3270/dialogs.h',
	'src/include/v3270/filetransfer.h',
	'src/include/v3270/ftprogress.h',
	'src/include/v3270/print.h',
	'src/include/v3270/security.h',
	'src/include/v3270/selection.h',
	'src/include/v3270/settings.h',
	'src/include/v3270/toggle.h',
	'src/include/v3270/tools.h',
	'src/include/v3270/trace.h',
	subdir: 'v3270'  
)

install_data(
	'remap/bracket.xml',
	install_dir: get_option('datadir') + '/' + lib3270.get_variable('product_name') + '/remap'
)

install_data(
	'conf/colors.conf',
	install_dir: get_option('datadir') + '/' + lib3270.get_variable('product_name')
)

