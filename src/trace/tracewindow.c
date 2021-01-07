/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como trace.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * SECTION:V3270TraceWindow
 * @Short_description: Window containing a trace monitor widget.
 * @Title: V3270Trace
 *
 * The #V3270TraceWindow shows a window with the lib3270 trace output.
 *
 */

 #include <config.h>
 #include <gtk/gtk.h>

 #define ENABLE_NLS

 #include <libintl.h>
 #include <glib/gi18n-lib.h>

 #include <v3270.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/properties.h>
 #include <v3270/trace.h>
 #include <internals.h>

#if defined( HAVE_SYSLOG )
 #include <syslog.h>
#endif // HAVE_SYSLOG

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void menu_item_new(GtkWidget *menu, const gchar *label, GCallback callback, gpointer data)
 {
	GtkWidget *widget = gtk_menu_item_new_with_mnemonic(label);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),widget);
	g_signal_connect(G_OBJECT(widget), "activate", callback, data);
 }

 static void menu_save(G_GNUC_UNUSED GtkWidget *button, GtkWidget *trace)
 {
	v3270_trace_save(trace);
 }

 static void menu_save_as(G_GNUC_UNUSED GtkWidget *button, GtkWidget *trace)
 {
	v3270_trace_select_file(trace);
 }

 static void menu_close(G_GNUC_UNUSED GtkWidget *button, GtkWidget *window)
 {
	debug("%s",__FUNCTION__);
	gtk_widget_destroy(window);
 }

 LIB3270_EXPORT GtkWidget * v3270_trace_window_new(GtkWidget *widget, const gchar *header)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

 	GtkWidget 	* window	= gtk_window_new(GTK_WINDOW_TOPLEVEL);
 	GtkWidget 	* vbox		= gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
 	GtkWidget	* trace		= v3270_trace_new(widget);

	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(window),TRUE);

 	// Set window title and default size
 	{
		const gchar 		* url 	= lib3270_get_url(v3270_get_session(widget));
		g_autofree gchar 	* title = NULL;

		if(url)
			title = g_strdup_printf("%s - %s - Trace", v3270_get_session_name(widget), url);
		else
			title = g_strdup_printf("%s - Trace", v3270_get_session_name(widget));

		gtk_window_set_title(GTK_WINDOW(window), title);
		gtk_window_set_default_size(GTK_WINDOW(window),590,430);
 	}

	// Top menu
	{
		GtkWidget * menubar = gtk_menu_bar_new();
		GtkWidget * topitem;
		GtkWidget * submenu;

		{
			// File menu
			topitem = gtk_menu_item_new_with_mnemonic(_("_File"));
			submenu = gtk_menu_new();

			gtk_menu_item_set_submenu(GTK_MENU_ITEM(topitem), submenu);
			gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topitem);

			menu_item_new(submenu,_("_Save"),G_CALLBACK(menu_save),trace);
			menu_item_new(submenu,_("Save _As"),G_CALLBACK(menu_save_as),trace);
			menu_item_new(submenu,_("_Close"),G_CALLBACK(menu_close),window);
		}

		gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE,TRUE,0);
	}

	// Trace window
	gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(trace),TRUE,TRUE,0);

	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_widget_show_all(window);

	if(header)
		v3270_trace_append_text(trace,header);

 	return window;
 }

