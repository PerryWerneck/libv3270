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
 #include <stdlib.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

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

static void trace_window_destroy(G_GNUC_UNUSED GtkWidget *widget, H3270 *hSession) {
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_DS_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_SCREEN_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_EVENT_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_NETWORK_TRACE,0);
}

static void activate(GtkApplication* app, G_GNUC_UNUSED gpointer user_data) {

	/*
	const gchar * search[]	= {

		g_get_user_config_dir(),
 		g_get_user_data_dir(),
 		g_get_home_dir(),
 		NULL

	};
	*/

	GtkWidget	* window	= gtk_application_window_new(app);
	GtkWidget	* terminal	= v3270_new();
//	gchar 		* filename	= NULL;
	GValue 		  val		= G_VALUE_INIT;

	GtkWidget *trace = v3270_new_trace_window(terminal);
	if(trace) {
		g_signal_connect(trace, "destroy", G_CALLBACK(trace_window_destroy), v3270_get_session(terminal));
		lib3270_toggle(v3270_get_session(terminal),LIB3270_TOGGLE_SSL_TRACE);
		lib3270_toggle(v3270_get_session(terminal),LIB3270_TOGGLE_DS_TRACE);
		gtk_widget_show_all(trace);
	}


	/*
	v3270_set_url(terminal,url);
	v3270_connect(terminal);
	*/

	/*
	g_value_init (&val, G_TYPE_STRING);
	g_value_set_string(&val,url);
	g_object_set_property(G_OBJECT(terminal), "url", &val);
	g_value_unset(&val);
	*/

	g_value_init(&val, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(terminal),"url",&val);
	g_message("URL=%s",g_value_get_string(&val));

	gchar * title = g_strdup_printf("%s - %s", v3270_get_session_name(terminal), g_value_get_string(&val));
	gtk_window_set_title(GTK_WINDOW(window), title);
	g_free(title);
	g_value_unset(&val);


	g_value_init(&val, G_TYPE_BOOLEAN);
	g_object_get_property(G_OBJECT(terminal),"tso",&val);
	g_message("TSO=%s",g_value_get_boolean(&val) ? "Yes" : "No");
	g_value_unset(&val);

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_object_get_property(G_OBJECT(terminal),"monocase",&val);
	g_message("monocase=%s",g_value_get_boolean(&val) ? "Yes" : "No");
	g_value_unset(&val);

	g_value_init(&val, G_TYPE_INT);
	g_object_get_property(G_OBJECT(terminal),"width",&val);
	g_message("width=%d",(int) g_value_get_int(&val));
	g_value_unset(&val);

	g_signal_connect(terminal,"popup",G_CALLBACK(popup_menu),NULL);

	// Setup and show window
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_container_add(GTK_CONTAINER(window),terminal);
	gtk_widget_show_all (window);

	v3270_set_toggle(terminal,LIB3270_TOGGLE_RECONNECT,1);

	// v3270_set_script(terminal,'R');

}

int main (int argc, char **argv) {

  GtkApplication *app;
  int status;

  app = gtk_application_new ("br.com.bb.pw3270",G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_message("rc=%d",status);
  return status;
}

