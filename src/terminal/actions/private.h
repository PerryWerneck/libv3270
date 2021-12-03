/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
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

 #include <config.h>

 #include <glib.h>
 #include <gtk/gtk.h>

 #include <v3270.h>
 #include <v3270/actions.h>
 #include <internals.h>

 // Lib3270 action

 G_GNUC_INTERNAL GType Lib3270Action_get_type(void) G_GNUC_CONST;

 #define LIB3270_TYPE_ACTION		(Lib3270Action_get_type())
 #define LIB3270_ACTION(inst)		(G_TYPE_CHECK_INSTANCE_CAST ((inst), LIB3270_TYPE_ACTION, Lib3270Action))
 #define LIB3270_IS_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), LIB3270_TYPE_ACTION))

 typedef struct _Lib3270ActionClass {
 	V3270ActionClass parent_class;
 } Lib3270ActionClass;

 typedef struct _Lib3270Action {
 	V3270Action parent;
 	const LIB3270_PROPERTY * definition;
 } Lib3270Action;


 // Internal methods
 G_GNUC_INTERNAL int fire_zoom_action(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_save_action(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_print_action(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_keypad_action(GtkWidget *widget, const V3270_ACTION *action);

