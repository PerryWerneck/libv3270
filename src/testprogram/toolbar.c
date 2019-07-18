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
 #include <v3270/trace.h>
 #include <lib3270/log.h>
 #include <stdlib.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

 static void print_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	lib3270_print_all(v3270_get_session(terminal));
 }

 static void host_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	v3270_select_host(terminal);
 }

 static void connect_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	lib3270_reconnect(v3270_get_session(terminal),0);
 }

 static void disconnect_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	lib3270_disconnect(v3270_get_session(terminal));
 }

 static void ft_clicked(GtkButton *button, GtkWidget *terminal)
 {
	/*
	GtkWidget * dialog	= v3270_dialog_new(terminal, _("test"), _("test"));
	GtkWidget * trace = v3270_trace_new(terminal);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),trace,TRUE,TRUE,2);
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

 	//
 	// Test settings dialog
 	//
	// GtkWidget * dialog = v3270_ft_settings_dialog_new(terminal);
	// v3270_ft_settings_dialog_append_activity(dialog,activity,NULL);

	//
	// V5.1 dialog
	//
	GtkWidget *dialog = v3270ft_new(GTK_WIDGET(button));

	/*
	//
	// Test worker widget
	//
	GtkWidget * dialog	= v3270_dialog_new(terminal, _("test"), _("test"));
	GtkWidget * worker = v3270_ft_worker_new();
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),worker,TRUE,TRUE,2);
	v3270_ft_worker_set_session(worker,v3270_get_session(terminal));
	v3270_ft_worker_set_activity(worker,activity);
	v3270_ft_worker_start(worker);
	*/

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

 }

 static void paste_clicked(GtkButton *button, GtkWidget *terminal)
 {
	v3270_paste(terminal);
 }

 static void copy_clicked(GtkButton *button, GtkWidget *terminal)
 {
	v3270_copy_selection(terminal,V3270_SELECT_TEXT,FALSE);
 }

 static void color_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal)
 {
	GtkWidget * dialog	= v3270_dialog_new(terminal, _("Color setup"), _("_Save"));
	GtkWidget * colors = v3270_color_selection_new(terminal);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),colors,TRUE,TRUE,2);

	gtk_widget_show_all(dialog);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		g_message("Accepted");
	}
	else
	{
		g_message("Cancel");
		v3270_color_selection_reset(colors);
	}

	gtk_widget_destroy(dialog);


 }

 GtkWidget * create_toolbar(GtkWidget *terminal) {

	size_t f;

	// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
	static const struct _buttons {
		const gchar * icon;
		GCallback	  callback;
		const gchar * tooltip;
	} buttons[] = {
		{ "gtk-connect",		G_CALLBACK(connect_clicked),	"Connect to host"  				},
		{ "gtk-disconnect",		G_CALLBACK(disconnect_clicked),	"Disconnect from host"  		},
		{ "gtk-select-color",	G_CALLBACK(color_clicked),		"Edit or change color scheme"	},
		{ "gtk-home",			G_CALLBACK(host_clicked),		"Configure host"				},
		{ "gtk-print",			G_CALLBACK(print_clicked),		"Print screen contents"			},
		{ "gtk-harddisk",		G_CALLBACK(ft_clicked),			"Open file transfer dialog"		},
		{ "gtk-copy",			G_CALLBACK(copy_clicked),		"Copy data"						},
		{ "gtk-paste",			G_CALLBACK(paste_clicked),		"Paste data"					}
	};

	GtkWidget * toolbar = gtk_toolbar_new();
	for(f = 0; f < G_N_ELEMENTS(buttons); f++)
	{
		GtkWidget * button = GTK_WIDGET(gtk_tool_button_new_from_stock(buttons[f].icon));
		gtk_widget_set_tooltip_markup(button,buttons[f].tooltip);
		g_signal_connect(G_OBJECT(button),"clicked",buttons[f].callback,terminal);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(button),-1);
		gtk_widget_set_can_focus(button,FALSE);
		gtk_widget_set_can_default(button,FALSE);
		gtk_widget_set_focus_on_click(button,FALSE);
	}
	gtk_widget_set_can_focus(toolbar,FALSE);

	return toolbar;

 }


