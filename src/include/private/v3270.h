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
  * @brief Declares v3270 private methods.
  */

 #pragma once
 #include <config.h>
 #include <gtk/gtk.h>

 // Signals
 enum {
  ACTIVATE,
 };

 // V3270 internal Properties
 enum {
  PROP_0,
 };

 typedef struct _V3270Private V3270Private;

 struct _V3270Private {
	int dunno;
 };

 // GObject methods.
 void v3270_editable_init(GtkEditableInterface *iface);
 void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
 void v3270_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 void v3270_finalize(GObject *object);
 void v3270_dispose(GObject *object);

 // Acessible
 void v3270_accessible_interface_init(GtkAccessibleInterface *iface);



