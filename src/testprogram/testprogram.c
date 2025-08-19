/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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

 #include "private.h"
 #include <v3270/filetransfer.h>
 #include <v3270/ftprogress.h>
 #include <v3270/colorscheme.h>
 #include <v3270/settings.h>
 #include <v3270/trace.h>
 #include <lib3270/log.h>
 #include <lib3270/properties.h>
 #include <v3270/actions.h>
 #include <stdlib.h>
 #include <gdk/gdkkeysyms-compat.h>
 #include <locale.h>
 #include <libintl.h>
 #include <v3270/selection.h>

 #if !GTK_CHECK_VERSION(2,74,0)
 	#define G_APPLICATION_DEFAULT_FLAGS 0
 #endif 

 /*---[ Implement ]----------------------------------------------------------------------------------*/

 static void session_changed(GtkWidget *terminal, GtkWidget *window) {
	gtk_window_set_title(GTK_WINDOW(window),v3270_get_session_title(terminal));
 }

 /*
 static gboolean field_clicked(GtkWidget *widget, gboolean connected, V3270_OIA_FIELD field, GdkEventButton *event, GtkWidget *window) {
	debug("%s: %s field=%d event=%p window=%p",__FUNCTION__,connected ? "Connected" : "Disconnected", field, event, window);

	if(!connected)
		return FALSE;

	if(field == V3270_OIA_SSL) {
		// v3270_popup_security_dialog(widget);
		debug("%s: Show SSL connection info dialog",__FUNCTION__);
		return TRUE;
	}

	return FALSE;
 }
 */

 static GKeyFile * get_key_file()
 {
	GKeyFile * key_file = g_key_file_new();
	g_key_file_load_from_file(key_file,"terminal.conf",G_KEY_FILE_NONE,NULL);
	return key_file;
 }

 static void save_settings(GtkWidget *terminal, GtkWidget *window)
 {
 	debug("%s: Saving settings for window %p",__FUNCTION__,window);

 	GKeyFile * key_file = get_key_file();

	v3270_to_key_file(terminal,key_file,"terminal");
	v3270_accelerator_map_to_key_file(terminal, key_file, "accelerators");

	g_key_file_save_to_file(key_file,"terminal.conf",NULL);
	g_key_file_free(key_file);


 }

 static void activate(GtkApplication* app, G_GNUC_UNUSED gpointer user_data) {

	GtkWidget	* window	= gtk_application_window_new(app);
	GtkWidget	* terminal	= v3270_new();
	GtkWidget	* vBox		= gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
	GtkWidget	* notebook	= gtk_notebook_new();

	// Hack to speed up the tests.
	//lib3270_ssl_set_crl_download(v3270_get_session(terminal),0);

	gtk_box_pack_start(GTK_BOX(vBox),create_toolbar(terminal),FALSE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vBox),notebook,TRUE,TRUE,0);

	// Create Terminal window
	{
		gtk_widget_set_can_default(terminal,TRUE);

#if GTK_CHECK_VERSION(3,20,0)
		gtk_widget_set_focus_on_click(terminal,TRUE);
#endif // GTK 3,20,0

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),terminal,gtk_label_new("Terminal"));

		// Load settings before connecting the signals.
		debug("%s: Loading settings...",__FUNCTION__);
		GKeyFile * key_file = get_key_file();
		v3270_load_key_file(terminal,key_file,NULL);
		v3270_accelerator_map_load_key_file(terminal,key_file,NULL);
		g_key_file_free(key_file);

	}

	// Create trace window
	v3270_set_trace(terminal,TRUE);

	// Setup and show main window
	g_autofree gchar * title = v3270_get_session_title(terminal);
	gtk_window_set_title(GTK_WINDOW(window),title);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_container_add(GTK_CONTAINER(window),vBox);
	gtk_widget_show_all(window);

	g_signal_connect(G_OBJECT(terminal),"session_changed",G_CALLBACK(session_changed),window);
	g_signal_connect(G_OBJECT(terminal),"save-settings",G_CALLBACK(save_settings),window);
	// g_signal_connect(G_OBJECT(terminal),"field_clicked",G_CALLBACK(field_clicked),window);

	gtk_widget_grab_focus(terminal);

}

int main (int argc, char **argv) {

	GtkApplication *app;
	int status;

	// Setup locale
#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

#ifdef _WIN32
	// https://stackoverflow.com/questions/37035936/how-to-get-native-windows-decorations-on-gtk3-on-windows-7-and-msys2
	putenv("GTK_CSD=0");
#endif // _WIN32

	bind_textdomain_codeset("libv3270", "UTF-8");
	textdomain("libv3270");

	g_message("Creating application...");
	app = gtk_application_new ("br.app.pw3270.v3270",G_APPLICATION_DEFAULT_FLAGS);

	g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

	g_message("Running application...");
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	g_message("rc=%d",status);

	return 0;

}


