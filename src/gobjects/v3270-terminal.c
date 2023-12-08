/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * v3270:
 *
 * `v3270` is tn3270 terminal emulator widget.
 *
 * ![An example v3270](v3270.png)
 *
 * `v3270` uses the %GTK_ACCESSIBLE_ROLE_TEXT_BOX role.
 *
 */

 // Reference:
 // https://gitlab.gnome.org/GNOME/gtk/-/blob/main/gtk/gtkentry.c
 // https://blog.gtk.org/2020/04/23/custom-widgets-in-gtk-4-introduction/
 // https://docs.gtk.org/gtk4/migrating-3to4.html

 #include <config.h>
 #include <gtk/gtk.h>
 #include <private/v3270>

 G_DEFINE_TYPE_WITH_CODE (V3270Terminal, v3270_terminal, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (V3270_terminal)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE,
                                                v3270_terminal_accessible_interface_init))
