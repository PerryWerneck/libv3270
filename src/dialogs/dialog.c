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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */


 #include "private.h"
 #include <internals.h>
 #include <lib3270.h>
 #include <lib3270/selection.h>
 #include <clipboard.h>
 #include <limits.h>
 #include <v3270/dialogs.h>


/*--[ Implement ]------------------------------------------------------------------------------------*/

 GtkWidget * v3270_dialog_new_with_buttons(const gchar *title, GtkWidget *widget, const gchar *first_button_text, ...) {

 	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

#ifdef _WIN32

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", FALSE,
			NULL
		));

#elif GTK_CHECK_VERSION(3,12,0)

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (v3270_dialog_get_use_header() ? 1 : 0),
			NULL
		));

#else

	GtkWidget * dialog = GTK_WIDGET(g_object_new(GTK_TYPE_DIALOG, NULL));

#endif	// GTK 3.12

	// Setup visual elements
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);

#ifdef G_OS_UNIX
	gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),18);
#else
	gtk_box_set_spacing(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),3);
#endif // UNIX

	// Setup window
	gtk_window_set_title(GTK_WINDOW(dialog),title);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);


	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);

	// Add buttons
	const gchar* text;
	gint response_id;

	va_list args;
	va_start (args, first_button_text);

	text = first_button_text;
	response_id = va_arg (args, gint);
	while(text != NULL) {

		gtk_dialog_add_button(GTK_DIALOG(dialog), text, response_id);

		text = va_arg (args, gchar*);
		if (text == NULL)
			break;
		response_id = va_arg (args, int);
	}

	va_end(args);

	return dialog;

 }

 GtkWidget * v3270_dialog_set_content_area(GtkWidget *dialog, GtkWidget *content_area) {

	if(GTK_IS_GRID(content_area)) {
		gtk_grid_set_row_spacing(GTK_GRID(content_area),6);
		gtk_grid_set_column_spacing(GTK_GRID(content_area),12);
	}

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),content_area,TRUE,TRUE,0);
	return content_area;
 }


