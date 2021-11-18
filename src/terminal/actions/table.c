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
 #include <lib3270/actions.h>
 #include <v3270/actions.h>
 #include <v3270/selection.h>
 #include <v3270/settings.h>
 #include <terminal.h>

 static int fire_copy_as_html(GtkWidget *widget, const struct _v3270_action * action);
 static int fire_copy_as_pixbuff(GtkWidget *widget, const struct _v3270_action * action);
 static int fire_accelerators_dialog(GtkWidget *widget, const struct _v3270_action * action);
 static int fire_host_dialog(GtkWidget *widget, const struct _v3270_action * action);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT const V3270_ACTION * v3270_get_actions() {

	 static const V3270_ACTION actions[] = {

		//
		// Standard Clipboard actions
		//
		{
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_SMART,
			.name = "copy",
			.keys = "<Primary>c",
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-copy",
			.label = N_( "Copy" ),
			.summary = N_("Copy selection to clipboard"),
			.description = N_("Replace current clipboard contents with the selected area"),
			.activate = fire_copy_accelerator
		},

		{
			.name = "copy-append",
			.keys = "<Primary><Shift>c",
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_APPEND,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Add to copy" ),
			.summary = N_("Append selection to clipboard"),
			.description = N_("Append selected area to current clipboard contents"),
			.activate = fire_copy_accelerator
		},

		{
			.name = "copy-html",
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Copy as HTML" ),
			.summary = N_("Copy selection in HTML format"),
			.description = N_("Replace current clipboard contents with the selected area in HTML format"),
			.activate = fire_copy_as_html
		},

		{
			.name = "copy-pixbuff",
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Copy as image" ),
			.summary = N_("Copy selection in image format"),
			.description = N_("Replace current clipboard contents with the selected area in image format"),
			.activate = fire_copy_as_pixbuff
		},

		{
			.name = "copy-text",
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_TEXT,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-copy",
			.label = N_( "Copy" ),
			.summary = N_( "Copy as plain text" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "copy-table",
			.keys = "<Shift><Alt>c",
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_TABLE,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-copy",
			.label = N_( "Copy as table" ),
			.summary = N_( "Copy as table" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut",
			.keys = "<Primary>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_SMART,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-cut",
			.label = N_( "Cut" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut-append",
			.keys = "<Alt>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_APPEND,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Cut" ),
			.summary = N_( "Cut and append to copy" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut-text",
			.keys = "<Primary><Shift>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TEXT,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-cut",
			.label = N_( "Cut" ),
			.summary = N_( "Cut as plain text" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut-table",
			.keys = "<Shift><Alt>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TABLE,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-cut",
			.label = N_( "Cut as table" ),
			.summary = N_( "Cut as table" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "paste",
			.keys = "<Primary>v",
			.flags = 0,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.icon = "edit-paste",
			.label = N_("Paste"),
			.summary = N_("Paste clipboard contents"),
			.description = N_("Input current clipboard contents to screen"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "paste-text",
			.keys = "<Primary><Shift>v",
			.flags = 1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.icon = "edit-paste",
			.label = N_("Paste"),
			.summary = N_("Paste as plain text"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "paste-file",
			.keys = "<Alt>v",
			.flags = 2,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Paste file"),
			.summary = N_("Paste from text file"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "paste-screen",
			.keys = "<Primary><Shift><Alt>v",
			.flags = 3,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Paste formatted screen"),
			.summary = N_("Paste similar screen from clipboard"),
			.description = N_("Search clipboard for a similar screen, if found paste unprotected fields and restore cursor position"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "zoom-in",
			.keys = "<Primary>KP_Add",
			.flags = 0,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.icon = "zoom-in",
			.label = N_("Zoom in"),
			.summary = N_("Increase the font size"),
			.activate = fire_zoom_action
		},

		{
			.name = "zoom-out",
			.keys = "<Primary>KP_Subtract",
			.flags = 1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Zoom out"),
			.summary = N_("decrease the font size"),
			.icon = "zoom-out",
			.activate = fire_zoom_action
		},

		{
			.name = "zoom-fit-best",
			.keys = "<Primary>0",
			.flags = 2,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Fit best"),
			.summary = N_("Set the font to the best size for window"),
			.icon = "zoom-fit-best",
			.activate = fire_zoom_action
		},

		//
		// Save actions
		//
		{
			.flags = -1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "save",
			.icon = "document-save-as",
			.label = N_("Save"),
			.summary = N_("Save screen or selection"),
			.activate = fire_save_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_ALL,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "save-all",
			.label = N_("Save all"),
			.icon = "document-save-as",
			.summary = N_("Save screen"),
			.activate = fire_save_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_SELECTED,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.name = "save-selected",
			.label = N_("Save selected"),
			.icon = "document-save-as",
			.summary = N_("Save selected area"),
			.activate = fire_save_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_COPY,
			.group = LIB3270_ACTION_GROUP_COPY,
			.name = "save-copy",
			.label = N_("Save copy"),
			.icon = "document-save-as",
			.summary = N_("Save Copy"),
			.activate = fire_save_action

		},

		//
		// Print actions
		//
		{
			.flags = -1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "print",
			.icon = "document-print",
			.label = N_("Print"),
			.summary = N_("Print screen or selection"),
			.activate = fire_print_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_ALL,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "print-all",
			.icon = "document-print",
			.label = N_("Print screen"),
			.summary = N_("Print the entire screen"),
			.activate = fire_print_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_SELECTED,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.name = "print-selected",
			.icon = "document-print",
			.label = N_("Print selected"),
			.summary = N_("Print selected area"),
			.activate = fire_print_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_COPY,
			.group = LIB3270_ACTION_GROUP_COPY,
			.name = "print-copy",
			.icon = "document-print",
			.label = N_("Print Copy"),
			.activate = fire_print_action

		},

		//
		// Dialog actions
		//
		{
			.group = LIB3270_ACTION_GROUP_NONE,
			.name = "dialog-keyboard",
			.label = N_("Change keyboard accelerators"),
			.activate = fire_accelerators_dialog

		},

		{
			.group = LIB3270_ACTION_GROUP_OFFLINE,
			.name = "dialog-host",
			.label = N_("Change host settings"),
			.activate = fire_host_dialog

		},

		//
		// Terminator
		//
		{
			.name = NULL
		}
	 };

	return actions;

 }

 static int fire_copy_as_html(GtkWidget *widget, const struct _v3270_action G_GNUC_UNUSED(* action)) {
	v3270_copy_as_html(widget);
	return 0;
 }

 static int fire_copy_as_pixbuff(GtkWidget *widget, const struct _v3270_action G_GNUC_UNUSED(* action)) {
	v3270_copy_as_pixbuff(widget);
	return 0;
 }

 static int fire_accelerators_dialog(GtkWidget *widget, const struct _v3270_action * action) {

	gtk_widget_show_all(
		v3270_settings_popup_dialog(
			v3270_accelerator_settings_new(),
			widget,
			TRUE
		)
	);

	return 0;
 }

 static int fire_host_dialog(GtkWidget *widget, const struct _v3270_action * action) {

	gtk_widget_show_all(
		v3270_settings_popup_dialog(
			v3270_host_settings_new(),
			widget,
			TRUE
		)
	);

	return 0;

	/*
	GtkWidget * dialog = v3270_settings_dialog_new();
	GtkWidget * settings = v3270_host_settings_new();

	v3270_settings_dialog_set_has_subtitle(dialog,FALSE);

	gtk_window_set_title(GTK_WINDOW(dialog), v3270_settings_get_title(settings));
	gtk_container_add(GTK_CONTAINER(dialog), settings);

	gtk_dialog_set_toplevel(dialog,widget);
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, widget);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 150);
	gtk_widget_show_all(dialog);

	gboolean again = TRUE;
 	while(again)
 	{
 		gtk_widget_set_sensitive(dialog,TRUE);
		gtk_widget_set_visible(dialog,TRUE);

 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_APPLY:
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_APPLY");
			v3270_settings_dialog_apply(dialog);
			again = lib3270_reconnect(v3270_get_session(widget),0);
			break;

		case GTK_RESPONSE_CANCEL:
			again = FALSE;
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_CANCEL");
			v3270_settings_dialog_revert(dialog);
			break;
 		}
 	}

	gtk_widget_destroy(dialog);
	*/

 }
