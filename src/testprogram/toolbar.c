/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <v3270.h>
 #include <v3270/filetransfer.h>
 #include <v3270/ftprogress.h>
 #include <v3270/colorscheme.h>
 #include <v3270/dialogs.h>
 #include <v3270/settings.h>
 #include <v3270/selection.h>
 #include <v3270/trace.h>
 #include <lib3270/log.h>
 #include <lib3270/popup.h>
 #include <stdlib.h>

 #pragma GCC diagnostic ignored "-Wunused-parameter"

 /*---[ Implement ]----------------------------------------------------------------------------------*/

 static void print_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
 	v3270_print(terminal,NULL);
 }

 static void host_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_select_host(terminal);
 }

 static void font_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_settings_popup_dialog(
		v3270_font_settings_new(),
		terminal,
		FALSE
	);

 }

 static void preferences_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
 	size_t ix;
 	GtkWidget * dialog = v3270_settings_dialog_new();

 	gtk_window_set_title(GTK_WINDOW(dialog),"Session properties");

 	GtkWidget * widgets[] = {
		v3270_host_settings_new(),
		v3270_color_settings_new(),
		v3270_font_settings_new(),
		v3270_accelerator_settings_new(),
		v3270_clipboard_settings_new()
 	};

 	for(ix = 0; ix < G_N_ELEMENTS(widgets); ix++) {
		gtk_container_add(GTK_CONTAINER(dialog), widgets[ix]);
 	}

	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(terminal)));

	v3270_settings_dialog_set_terminal_widget(dialog, terminal);

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(v3270_setttings_dialog_response),NULL);

	gtk_widget_show_all(dialog);

 }

 static void connect_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	lib3270_reconnect(v3270_get_session(terminal),0);
 }

 static void disconnect_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	lib3270_disconnect(v3270_get_session(terminal));
 }

 static void save_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
 	H3270 *hSession = v3270_get_session(terminal);
 	if(lib3270_get_has_selection(hSession))
		lib3270_save_selected(hSession,NULL);
	else
		lib3270_save_all(hSession,NULL);
 }

 static void load_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
 	lib3270_load(v3270_get_session(terminal),NULL);
 }

 static void ft_clicked(GtkButton *button, GtkWidget *terminal)
 {
 	// v3270_error_popup(terminal,"title","summary","body");

 	/*
 	{
		GtkWidget * dialog = v3270_dialog_new_with_buttons("Testing dialog", terminal, "Ok", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);

		v3270_dialog_set_content_area(dialog,gtk_label_new("test label"));

		g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);
		gtk_widget_show_all(dialog);

 	}
 	*/


	/*
	//
	// Test transfer dialog.
	//
	v3270_transfer_file(
		terminal,
		LIB3270_FT_OPTION_RECEIVE|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_REMAP,
		"/tmp/test.txt",
		"remote_file",
		0,
		0,
		0,
		0,
		4096
	);
	*/

	/*
	//
	// Test activity
	//
	GObject * activity = v3270_ft_activity_new();
 	debug("Activity %p is %s",activity,(g_object_is_floating(activity) ? "floating" : "non floating"));
	v3270_ft_activity_set_local_filename(activity,"/tmp/test.txt");
	v3270_ft_activity_set_remote_filename(activity,"remote_file");
	v3270_ft_activity_set_options(activity,LIB3270_FT_OPTION_RECEIVE|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_REMAP);
	*/


	/*
	{
		//
		// Test V3270 FT Dialog
		//
		//
		// GtkWidget * dialog = v3270_ft_settings_dialog_new(terminal);
		// v3270_ft_settings_dialog_append_activity(dialog,activity,NULL);

		//
		// V5.1 dialog
		//
		GtkWidget *dialog = v3270ft_new(GTK_WIDGET(button));

		g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);
		gtk_widget_show_all(dialog);

	}
	*/


 }

 static void popup_clicked(GtkButton *button, GtkWidget *terminal)
 {

		static const LIB3270_POPUP popup = {
		.name = "sample_popup",
		.type = LIB3270_NOTIFY_INFO,
		.summary = "This is the summary of message",
		.body = "This it the body of the message, can be used for a bigger explanation"
	};

	lib3270_popup_show(v3270_get_session(terminal), &popup, 1);

 }

 static void paste_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_clipboard_get_from_url(terminal,NULL);
 }

 static void copy_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_clipboard_set(terminal,V3270_COPY_TEXT,FALSE);
 }

 static void color_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_edit_color_table(terminal);
 }

 static void zoom_in_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_zoom_in(terminal);
 }

 static void zoom_out_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_zoom_out(terminal);
 }

 static void zoom_best_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_zoom_best(terminal);
 }

 GtkWidget * create_toolbar(GtkWidget *terminal) {

	size_t f;

	// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
	static const struct _buttons {
		const gchar * icon;			// https://specifications.freedesktop.org/icon-naming-spec/latest/ar01s04.html
		GCallback	  callback;
		const gchar * tooltip;
	} buttons[] = {
		{ "gtk-connect",				G_CALLBACK(connect_clicked),		"Connect to host"  				},
		{ "gtk-disconnect",				G_CALLBACK(disconnect_clicked),		"Disconnect from host"  		},
		{ "gtk-print",					G_CALLBACK(print_clicked),			"Print screen contents"			},
		{ "gtk-harddisk",				G_CALLBACK(ft_clicked),				"Open file transfer dialog"		},
		{ "gtk-copy",					G_CALLBACK(copy_clicked),			"Copy data"						},
		{ "gtk-paste",					G_CALLBACK(paste_clicked),			"Paste data"					},
		{ "document-save",				G_CALLBACK(save_clicked),			"Save screen or selection"		},
		{ "document-open",				G_CALLBACK(load_clicked),			"Paste file"					},
		{ "dialog-information",			G_CALLBACK(popup_clicked),			"Show test popup"				},

		{ "applications-system",		G_CALLBACK(preferences_clicked),	"Session properties"			},

//		{ "network-server",				G_CALLBACK(host_clicked),			"Configure host"				},
//		{ "preferences-desktop-font",	G_CALLBACK(font_clicked),			"Select font"					},
		{ "gtk-select-color",			G_CALLBACK(color_clicked),			"Edit or change color scheme"	},


		{ "zoom-in",					G_CALLBACK(zoom_in_clicked),		"Zoom in"						},
		{ "zoom-out",					G_CALLBACK(zoom_out_clicked),		"Zoom out"						},
		{ "zoom-fit-best",				G_CALLBACK(zoom_best_clicked),		"Zoom best"						},
	};

	GtkWidget * toolbar = gtk_toolbar_new();
	for(f = 0; f < G_N_ELEMENTS(buttons); f++)
	{
		GtkWidget * button = GTK_WIDGET(gtk_tool_button_new(gtk_image_new_from_icon_name(buttons[f].icon,GTK_ICON_SIZE_SMALL_TOOLBAR),"-"));

		gtk_widget_set_tooltip_markup(button,buttons[f].tooltip);
		g_signal_connect(G_OBJECT(button),"clicked",buttons[f].callback,terminal);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(button),-1);
		gtk_widget_set_can_focus(button,FALSE);
		gtk_widget_set_can_default(button,FALSE);
#if GTK_CHECK_VERSION(3,20,0)
		gtk_widget_set_focus_on_click(button,FALSE);
#endif // GTK 3,20,0

	}
	gtk_widget_set_can_focus(toolbar,FALSE);

	return toolbar;

 }


