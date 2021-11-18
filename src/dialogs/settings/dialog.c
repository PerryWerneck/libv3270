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

 #include "../private.h"
 #include <internals.h>
 #include <v3270/settings.h>
 #include <terminal.h>
 #include <lib3270/log.h>
 #include <v3270/tools.h>

 struct _V3270SettingsDialog {
  	GtkDialog		  parent;
  	GtkNotebook		* tabs;
  	GtkWidget		* terminal;
  	gboolean		  has_subtitle;
 };

 struct _V3270SettingsDialogClass	{
 	GtkDialogClass parent_class;
 };

 G_DEFINE_TYPE(V3270SettingsDialog, V3270SettingsDialog, GTK_TYPE_DIALOG);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void on_switch_page(GtkNotebook G_GNUC_UNUSED(*notebook), V3270Settings *settings, guint G_GNUC_UNUSED(page_num), V3270SettingsDialog *dialog)
{
	debug("title: %s",settings->title);
	debug("label: %s",settings->label);

	if(dialog->has_subtitle) {
		GtkWidget * header_bar = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
		if(header_bar) {
			gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar),settings->title);
		}
	}

}

static void check_valid(GtkWidget *widget, gboolean *valid)
{
	if(*valid && GTK_IS_V3270_SETTINGS(widget))
	{
		if(!v3270_settings_get_valid(widget))
			*valid = FALSE;
	}

}

static void on_validity(V3270Settings G_GNUC_UNUSED(*settings), gboolean valid, V3270SettingsDialog * dialog)
{
	if(valid)
	{
		// Check validity of all childs.
		gtk_container_foreach(
			GTK_CONTAINER(dialog->tabs),
			(GtkCallback) check_valid,
			&valid
		);

	}

	debug("The current state is %s",valid ? "valid" : "invalid");

	GtkWidget * button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_APPLY);

	if(button)
		gtk_widget_set_sensitive(button,valid);


}

static void add(GtkContainer *container, GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(widget),18);

	GtkWidget * label = NULL;
	if(GTK_V3270_SETTINGS(widget)->label)
	{
		label = gtk_label_new(GTK_V3270_SETTINGS(widget)->label);

		if(GTK_V3270_SETTINGS(widget)->tooltip)
			gtk_widget_set_tooltip_markup(label,GTK_V3270_SETTINGS(widget)->tooltip);

	}

	gtk_notebook_append_page(
		GTK_V3270_SETTINGS_DIALOG(container)->tabs,
		widget,
		label
	);

//	g_signal_connect(G_OBJECT(widget), "focus-in-event", G_CALLBACK(on_tab_focus), container);
	g_signal_connect(G_OBJECT(widget), "validity", G_CALLBACK(on_validity), container);


}

static void apply_settings(GtkWidget *widget, GtkWidget G_GNUC_UNUSED(* terminal))
{
	if(GTK_IS_V3270_SETTINGS(widget))
		v3270_settings_apply(widget);
}

static void revert_settings(GtkWidget *widget, GtkWidget G_GNUC_UNUSED(* terminal))
{
	if(GTK_IS_V3270_SETTINGS(widget))
		v3270_settings_revert(widget);
}

void v3270_settings_dialog_apply(GtkWidget *dialog)
{
	debug("%s",__FUNCTION__);

	GtkWidget * terminal = GTK_V3270_SETTINGS_DIALOG(dialog)->terminal;
	if(!terminal)
		return;

	GdkWindow * window = gtk_widget_get_window(dialog);
	gdk_window_set_cursor(
		window,
		GTK_V3270_GET_CLASS(terminal)->cursors[LIB3270_POINTER_WAITING]
	);

	gdk_display_sync(gtk_widget_get_display(dialog));

	// Freeze to avoid multiple "save settings" signals.
	GTK_V3270(terminal)->freeze = 1;

	gtk_container_foreach(
		GTK_CONTAINER(GTK_V3270_SETTINGS_DIALOG(dialog)->tabs),
		(GtkCallback) apply_settings,
		terminal
	);

	// Don't delay save settings signal!
	GTK_V3270(terminal)->freeze = 0;
	v3270_signal_emit(terminal,V3270_SIGNAL_SAVE_SETTINGS, FALSE);

	gdk_window_set_cursor(
		window,
		NULL
	);

}

void v3270_settings_dialog_revert(GtkWidget *dialog)
{
	debug("%s",__FUNCTION__);

	GtkWidget * terminal = GTK_V3270_SETTINGS_DIALOG(dialog)->terminal;
	if(!terminal)
		return;

	gtk_container_foreach(
		GTK_CONTAINER(GTK_V3270_SETTINGS_DIALOG(dialog)->tabs),
		(GtkCallback) revert_settings,
		terminal
	);

}

static void V3270SettingsDialog_class_init(V3270SettingsDialogClass *klass)
{
	GTK_CONTAINER_CLASS(klass)->add = add;
}

static void on_page_changed(GtkNotebook *notebook, GtkWidget G_GNUC_UNUSED(*child), guint G_GNUC_UNUSED(page_num), V3270SettingsDialog G_GNUC_UNUSED(*dialog)) {
 	gtk_notebook_set_show_tabs(notebook,gtk_notebook_get_n_pages(notebook) > 1);
}

static void V3270SettingsDialog_init(V3270SettingsDialog *dialog)
{
	GtkWidget * content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	// Get use of header bar.
	dialog->has_subtitle = v3270_dialog_get_use_header();

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	gtk_dialog_add_buttons(
		GTK_DIALOG(dialog),
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Apply"), GTK_RESPONSE_APPLY,
		NULL
	);

	// Create notebook for settings widgets
	dialog->tabs = GTK_NOTEBOOK(gtk_notebook_new());

	gtk_notebook_set_scrollable(dialog->tabs,TRUE);
	gtk_notebook_set_show_tabs(dialog->tabs,FALSE);
	g_signal_connect(G_OBJECT(dialog->tabs), "page-added", G_CALLBACK(on_page_changed), dialog);
	g_signal_connect(G_OBJECT(dialog->tabs), "page-removed", G_CALLBACK(on_page_changed), dialog);
	g_signal_connect(G_OBJECT(dialog->tabs), "switch-page", G_CALLBACK(on_switch_page), dialog);
	gtk_box_pack_start(GTK_BOX(content_area),GTK_WIDGET(dialog->tabs),TRUE,TRUE,0);

	if(!dialog->has_subtitle) {
		gtk_box_set_spacing(GTK_BOX(content_area),3);
		gtk_widget_set_margin_bottom(content_area,3);
	}
}

GtkWidget * v3270_settings_dialog_new()
{
#if GTK_CHECK_VERSION(3,12,0)

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_V3270_SETTINGS_DIALOG,
			"use-header-bar", (v3270_dialog_get_use_header() ? 1 : 0),
			NULL
		));

#else

	GtkWidget * dialog = GTK_WIDGET(g_object_new(GTK_TYPE_V3270_SETTINGS_DIALOG, NULL));

#endif	// GTK 3.12

 	return dialog;

}

struct _set_terminal_info
{
	GtkNotebook *notebook;	///< @brief The notebook.
	GtkWidget *terminal;	///< @brief The V3270 terminal.
	gint selected;			///< @brief First sensitive page (-1 = none)
};

static void set_terminal_widget(GtkWidget *settings, struct _set_terminal_info *info)
{
	if(GTK_IS_V3270_SETTINGS(settings))
	{
		v3270_settings_set_terminal_widget(settings,info->terminal);
		if(gtk_widget_get_sensitive(settings) && info->selected < 0)
			info->selected = gtk_notebook_page_num(info->notebook,settings);
	}

	gtk_widget_show(settings);
}

void v3270_settings_dialog_set_terminal_widget(GtkWidget *widget, GtkWidget *terminal)
{
	g_return_if_fail(GTK_IS_V3270_SETTINGS_DIALOG(widget));

	debug("%s(%p,%p)",__FUNCTION__,widget,terminal);

	GTK_V3270_SETTINGS_DIALOG(widget)->terminal = terminal;

	struct _set_terminal_info info = {
		.notebook = GTK_V3270_SETTINGS_DIALOG(widget)->tabs,
		.terminal = terminal,
		.selected = -1
	};

	gtk_container_foreach(
		GTK_CONTAINER(info.notebook),
		(GtkCallback) set_terminal_widget,
		&info
	);

	if(info.selected >= 0)
	{
		debug("Found active page on %d",info.selected);
		gtk_notebook_set_current_page(info.notebook, info.selected);
	}
	else
	{
		g_message("No active page on settings dialog");
	}

	gtk_window_set_transient_for(
		GTK_WINDOW(widget),
		GTK_WINDOW(gtk_widget_get_toplevel(terminal))
	);

}

void v3270_settings_dialog_set_has_subtitle(GtkWidget *widget, gboolean has_subtitle)
{
	g_return_if_fail(GTK_IS_V3270_SETTINGS_DIALOG(widget));
	GTK_V3270_SETTINGS_DIALOG(widget)->has_subtitle = has_subtitle;
}

void v3270_setttings_dialog_response(GtkDialog *dialog, gint response_id, GtkWidget G_GNUC_UNUSED(*terminal))
{
    switch(response_id)
    {
    case GTK_RESPONSE_APPLY:
    	debug("%s:apply",__FUNCTION__);
		v3270_settings_dialog_apply(GTK_WIDGET(dialog));
        break;

    case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_DELETE_EVENT:
    	debug("%s:cancel",__FUNCTION__);
		v3270_settings_dialog_revert(GTK_WIDGET(dialog));
        break;

	default:
		g_warning("Unexpected settings dialog response \"%d\"",response_id);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));

}

GtkWidget * v3270_settings_get_edit_dialog(GtkWidget *settings, GtkWidget *terminal, gboolean modal) {

 	const gchar * title = GTK_V3270_SETTINGS(settings)->title;

 	GtkWidget * dialog = v3270_settings_dialog_new();

 	if(title)
		gtk_window_set_title(GTK_WINDOW(dialog), title);

	gtk_container_add(GTK_CONTAINER(dialog), settings);

	gtk_dialog_set_toplevel(dialog,terminal);
	gtk_window_set_modal(GTK_WINDOW(dialog),modal);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, terminal);

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(v3270_setttings_dialog_response),settings);

	return dialog;

 }

