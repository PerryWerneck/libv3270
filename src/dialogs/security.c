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

 #include <config.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <gtk/gtk.h>
 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <internals.h>

 #include <v3270.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static GtkWidget * label_new(const char *text)
 {
 	GtkLabel *widget = GTK_LABEL(gtk_label_new(""));
 	gtk_label_set_markup(widget,text);
#if GTK_CHECK_VERSION(3,16,0)
	gtk_label_set_xalign(widget,0);
#else
	gtk_misc_set_alignment(GTK_MISC(widget),0,0.5);
#endif // GTK_CHECK_VERSION

 	return GTK_WIDGET(widget);
 }

 static GtkWidget * text_view_new(const char *contents)
 {
	GtkWidget * container = gtk_scrolled_window_new(NULL,NULL);
	GtkWidget * view = gtk_text_view_new();

 	gtk_container_set_border_width(GTK_CONTAINER(container),5);
 	gtk_container_set_border_width(GTK_CONTAINER(view),5);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(container),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);

	GtkTextIter	itr;
	GtkTextBuffer * text = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)));

	gtk_text_buffer_get_end_iter(text,&itr);
	gtk_text_buffer_insert(text,&itr,contents,strlen(contents));

	gtk_container_add(GTK_CONTAINER(container),view);
	gtk_text_view_set_monospace(GTK_TEXT_VIEW(view),TRUE);

	return container;
 }

 LIB3270_EXPORT GtkWidget * v3270_security_dialog_new(GtkWidget *widget)
 {
	GtkGrid * grid = GTK_GRID(gtk_grid_new());
 	gtk_container_set_border_width(GTK_CONTAINER(grid),10);
 	gtk_grid_set_row_spacing(grid,5);
 	gtk_grid_set_column_spacing(grid,5);

	H3270 *hSession = NULL;

	if(GTK_IS_V3270(widget))
		hSession = v3270_get_session(widget);

	if(hSession)
	{
		gtk_grid_attach(
			grid,
			gtk_image_new_from_icon_name(lib3270_get_ssl_state_icon_name(hSession),GTK_ICON_SIZE_DIALOG),
			0,0,
			2,2
		);

		g_autofree gchar * message = g_strdup_printf("<b>%s</b>",lib3270_get_ssl_state_message(hSession));
		gtk_grid_attach(
			grid,
			label_new(message),
			3,0,
			6,1
		);

		gtk_grid_attach(
			grid,
			label_new(lib3270_get_ssl_state_description(hSession)),
			3,1,
			6,1
		);

		lib3270_autoptr(char) crl = lib3270_get_ssl_crl_text(hSession);
		lib3270_autoptr(char) peer = lib3270_get_ssl_peer_certificate_text(hSession);

		if(!peer)
			peer = g_strdup("");

		if(crl)
		{
			GtkWidget * frame =	gtk_notebook_new();

			gtk_widget_set_hexpand(GTK_WIDGET(frame),TRUE);
			gtk_widget_set_vexpand(GTK_WIDGET(frame),TRUE);

			gtk_notebook_append_page(
				GTK_NOTEBOOK(frame),
				text_view_new(peer),
				gtk_label_new(_("Peer certificate"))
			);

			gtk_notebook_append_page(
				GTK_NOTEBOOK(frame),
				text_view_new(crl),
				gtk_label_new(_("Revocation list"))
			);

			gtk_grid_attach(
				grid,
				frame,
				3,2,
				6,6
			);

		}
		else
		{
			GtkWidget * frame = gtk_frame_new(_("Peer certificate"));
			gtk_container_add(GTK_CONTAINER(frame),text_view_new(peer));

			gtk_widget_set_hexpand(GTK_WIDGET(frame),TRUE);
			gtk_widget_set_vexpand(GTK_WIDGET(frame),TRUE);

			gtk_grid_attach(
				grid,
				frame,
				3,2,
				6,6
			);


		}

	}
	else
	{

		gtk_grid_attach(
			grid,
			gtk_image_new_from_icon_name("dialog-error",GTK_ICON_SIZE_DIALOG),
			0,0,
			2,2
		);

		gtk_grid_attach(
			grid,
			label_new(_("Invalid state")),
			3,0,
			6,1
		);

		gtk_grid_attach(
			grid,
			label_new(_("There's no security information in the session")),
			3,1,
			6,1
		);
	}

	return GTK_WIDGET(grid);
 }

 LIB3270_EXPORT	void v3270_popup_security_dialog(GtkWidget *widget)
 {
 	GtkWidget * dialog = gtk_dialog_new_with_buttons(
								_("About security"),
								GTK_WINDOW(gtk_widget_get_toplevel(widget)),
								GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								_( "_Close" ), GTK_RESPONSE_ACCEPT,
								NULL
						);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 800, 500);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),v3270_security_dialog_new(widget),TRUE,TRUE,2);
	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));


 }
