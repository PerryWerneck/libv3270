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
 * Este programa está nomeado como testprogram.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 /**
  * @brief V3270 Widget test program.
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

	/*
static gboolean popup_menu(GtkWidget *widget, G_GNUC_UNUSED gboolean selected, gboolean online, G_GNUC_UNUSED GdkEventButton *event, G_GNUC_UNUSED gpointer user_data) {

	if(!online)
		return FALSE;

	GtkWidget * dialog = v3270ft_new();

	// v3270ft_load(dialog,"transfer.xml");

	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(widget)));

	do {

		gtk_widget_show_all(dialog);

		switch(gtk_dialog_run(GTK_DIALOG(dialog))) {
		case GTK_RESPONSE_APPLY:
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_YES:
			gtk_widget_hide(dialog);
			v3270ft_transfer(dialog,v3270_get_session(widget));
			break;

		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_NO:
		case GTK_RESPONSE_DELETE_EVENT:
			v3270ft_remove_all(dialog);
			break;

		default:
			g_warning("Unexpected response from v3270ft");
		}

	} while(v3270ft_get_length(dialog) > 0);

	gtk_widget_destroy(dialog);

	return TRUE;

}
	*/

 static gboolean field_clicked(GtkWidget *widget, gboolean connected, V3270_OIA_FIELD field, GdkEventButton *event, GtkWidget *window)
 {
	trace("%s: %s field=%d event=%p window=%p",__FUNCTION__,connected ? "Connected" : "Disconnected", field, event, window);

	if(!connected)
		return FALSE;

	if(field == V3270_OIA_SSL)
	{
		v3270_popup_security_dialog(widget);
		trace("%s: Show SSL connection info dialog",__FUNCTION__);
		return TRUE;
	}


	return FALSE;
 }

static void trace_window_destroy(G_GNUC_UNUSED GtkWidget *widget, H3270 *hSession) {
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_DS_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_SCREEN_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_EVENT_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_NETWORK_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_SSL_TRACE,0);
}


/*
static void color_scheme_changed(GtkWidget G_GNUC_UNUSED(*widget), const GdkRGBA *colors, GtkWidget *terminal) {

	debug("%s=%p",__FUNCTION__,colors);

	int f;
	for(f=0;f<V3270_COLOR_COUNT;f++)
		v3270_set_color(terminal,f,colors+f);

	v3270_reload(terminal);
	gtk_widget_queue_draw(terminal);

}
*/

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

static void toggle_ds_trace(GtkToggleToolButton *button, GtkWidget *terminal) {
	v3270_set_toggle(terminal,LIB3270_TOGGLE_DS_TRACE,gtk_toggle_tool_button_get_active(button));
}

static void toggle_event_trace(GtkToggleToolButton *button, GtkWidget *terminal) {
	v3270_set_toggle(terminal,LIB3270_TOGGLE_EVENT_TRACE,gtk_toggle_tool_button_get_active(button));
}

static void toggle_ssl_trace(GtkToggleToolButton *button, GtkWidget *terminal) {
	v3270_set_toggle(terminal,LIB3270_TOGGLE_SSL_TRACE,gtk_toggle_tool_button_get_active(button));
}

static void toggle_screen_trace(GtkToggleToolButton *button, GtkWidget *terminal) {
	v3270_set_toggle(terminal,LIB3270_TOGGLE_SCREEN_TRACE,gtk_toggle_tool_button_get_active(button));
}

static GtkToolItem * create_tool_item(GtkWidget *terminal, const gchar *label, const gchar *tooltip, GCallback callback) {
	GtkToolItem * item = gtk_toggle_tool_button_new();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(item),label);

	gtk_widget_set_can_focus(GTK_WIDGET(item),FALSE);
	gtk_widget_set_can_default(GTK_WIDGET(item),FALSE);
	gtk_widget_set_focus_on_click(GTK_WIDGET(item),FALSE);

	g_signal_connect(GTK_WIDGET(item), "toggled", G_CALLBACK(callback), terminal);

	if(tooltip)
		gtk_widget_set_tooltip_text(GTK_WIDGET(item),tooltip);

	return item;
}

static void activate(GtkApplication* app, G_GNUC_UNUSED gpointer user_data) {

	GtkWidget	* window	= gtk_application_window_new(app);
	GtkWidget	* terminal	= v3270_new();
	GtkWidget	* notebook	= gtk_notebook_new();
	size_t		  f;

	// Create Terminal window
	{
		GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,2);

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
			{ "gtk-harddisk",		G_CALLBACK(ft_clicked),			"Open file transfer dialog"		}
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

		gtk_box_pack_start(GTK_BOX(box),toolbar,FALSE,TRUE,0);

		gtk_widget_set_can_default(terminal,TRUE);
		gtk_widget_set_focus_on_click(terminal,TRUE);
		gtk_box_pack_start(GTK_BOX(box),terminal,TRUE,TRUE,0);

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),box,gtk_label_new("Terminal"));
	}

	// Create trace window
	{
		GtkWidget	* box		= gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		GtkWidget	* trace 	= v3270_trace_new(terminal);
		GtkWidget	* toolbar	= gtk_toolbar_new();

		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),create_tool_item(terminal, "DS Trace","Toggle DS Trace",G_CALLBACK(toggle_ds_trace)),-1);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),create_tool_item(terminal, "Event Trace","Toggle Event Trace",G_CALLBACK(toggle_event_trace)),-1);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),create_tool_item(terminal, "Screen Trace","Toggle Screen Trace",G_CALLBACK(toggle_screen_trace)),-1);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),create_tool_item(terminal, "SSL Trace","Toggle SSL Trace",G_CALLBACK(toggle_ssl_trace)),-1);

		gtk_box_pack_start(GTK_BOX(box),toolbar,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),trace,TRUE,TRUE,0);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),box,gtk_label_new("Trace"));
	}

	// Setup and show main window
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_container_add(GTK_CONTAINER(window),notebook);
	gtk_widget_show_all(window);

	gtk_widget_grab_focus(terminal);

}

int main (int argc, char **argv) {

  GtkApplication *app;
  int status;

  app = gtk_application_new ("br.com.bb.pw3270",G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_message("rc=%d",status);

  return 0;

}


