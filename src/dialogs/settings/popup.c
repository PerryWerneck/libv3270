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

 #include "private.h"
 #include <v3270/settings.h>
 #include <v3270/tools.h>

 GtkWidget * v3270_settings_popup_dialog(GtkWidget *settings, GtkWidget *terminal, gboolean modal)
 {
#ifdef DEBUG
	g_message("terminal=%d settings=%d",(int) GTK_IS_V3270(terminal), (int) GTK_IS_V3270_SETTINGS(settings));
#endif // DEBUG
	if( !(GTK_IS_V3270(terminal) && GTK_IS_V3270_SETTINGS(settings))) {
		g_message("%s was called with invalid arguments",__FUNCTION__);
		if(g_object_is_floating(settings)) {
			g_object_ref_sink(settings);
		}
		g_object_unref(settings);
		return NULL;
	}

	GtkWidget * dialog = v3270_settings_dialog_new();
	v3270_settings_dialog_set_has_subtitle(dialog,FALSE);

	gtk_window_set_title(GTK_WINDOW(dialog), v3270_settings_get_title(settings));
	gtk_container_add(GTK_CONTAINER(dialog), settings);

	gtk_dialog_set_toplevel(dialog,terminal);
	gtk_window_set_modal(GTK_WINDOW(dialog),modal);

	v3270_settings_dialog_set_terminal_widget(dialog, terminal);

	g_signal_connect(dialog,"response",G_CALLBACK(v3270_setttings_dialog_response),NULL);
	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);

	return dialog;

 }
