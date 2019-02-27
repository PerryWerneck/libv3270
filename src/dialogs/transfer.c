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
 #include <v3270/filetransfer.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void header_on_state_changed(GtkWidget *worker, guint state, const gchar *msg, GtkDialog *dialog)
{
	debug("%s(%s)",__FUNCTION__,msg);
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(gtk_dialog_get_header_bar(dialog)),msg);
}

static void transfer_success(GtkWidget *worker, const gchar *msg, const gchar *description, GtkDialog *dialog)
{
	debug("%s(\"%s\",\"%s\")",__FUNCTION__,msg,description);
	gtk_dialog_response(dialog,GTK_RESPONSE_OK);

}

/*
static void transfer_failed(GtkWidget *worker, const gchar *msg, const gchar *description, GtkDialog *dialog)
{
	debug("%s(\"%s\",\"%s\")",__FUNCTION__,msg,description);
}
*/

static void save_activity_clicked(GtkWidget *button, GObject *activity)
{
	debug("%s",__FUNCTION__);

	g_autofree gchar * filename = v3270_select_file(
								gtk_widget_get_toplevel(button),
								_("Save transfer activity to file"),
								_("Save"),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								NULL,
								N_("XML file"), "*.xml",
								NULL );

	if(filename)
	{
		GError	* error	= NULL;
		GString * str	= g_string_new("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<filelist>\n");

		v3270_ft_activity_xml_encode(activity,str);
		g_string_append(str,"</filelist>\n");

		g_autofree gchar * text = g_string_free(str,FALSE);

		debug("Saving %s",filename);

		if(!g_file_set_contents(filename,text,-1,&error)) {

			GtkWidget *popup = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(gtk_widget_get_toplevel(button)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
				_("Can't save %s"),filename
			);

			gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
			g_error_free(error);

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);

		}

	}

}

LIB3270_EXPORT gint v3270_transfer_file(GtkWidget *widget, LIB3270_FT_OPTION options, const gchar *local, const gchar *remote, int lrecl, int blksize, int primspace, int secspace, int dft)
{
	// Create activity
	GObject * activity = v3270_ft_activity_new();

	v3270_ft_activity_set_local_filename(activity,local);
	v3270_ft_activity_set_remote_filename(activity,remote);
	v3270_ft_activity_set_options(activity,options);

	v3270_ft_activity_set_value(activity,LIB3270_FT_VALUE_LRECL, lrecl);
	v3270_ft_activity_set_value(activity,LIB3270_FT_VALUE_BLKSIZE, blksize);
	v3270_ft_activity_set_value(activity,LIB3270_FT_VALUE_PRIMSPACE, primspace);
	v3270_ft_activity_set_value(activity,LIB3270_FT_VALUE_SECSPACE, secspace);
	v3270_ft_activity_set_value(activity,LIB3270_FT_VALUE_DFT, dft);

	// Create worker
	GtkWidget * worker = v3270_ft_worker_new();
	v3270_ft_worker_set_session(worker,v3270_get_session(widget));
	v3270_ft_worker_set_activity(worker,activity);

	// Create save button
	GtkWidget * save_button = NULL;

	// Create dialog
	const gchar *title = (options & LIB3270_FT_OPTION_RECEIVE) ? _( "Receiving file" ) : _( "Sending file" );
	gboolean use_header = FALSE;

#if GTK_CHECK_VERSION(3,12,0)

	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

	if(use_header)
	{
		GtkWidget * header = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
		gtk_header_bar_set_title(GTK_HEADER_BAR(header),title);
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header),_("Starting transfer"));

		save_button = gtk_button_new_from_icon_name("document-save-as",GTK_ICON_SIZE_SMALL_TOOLBAR);
		gtk_header_bar_pack_start(GTK_HEADER_BAR(header),save_button);

		g_signal_connect(worker,"ft-state-changed",G_CALLBACK(header_on_state_changed), dialog);

		gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),18);

	}
	else
	{
		gtk_window_set_title(GTK_WINDOW(dialog), title);
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

		gtk_container_set_border_width(GTK_CONTAINER(worker),18);

		gtk_dialog_add_buttons(
			GTK_DIALOG (dialog),
			_("_Close"), GTK_RESPONSE_CANCEL,
			NULL
		);

	}

#else

	GtkWidget * dialog = gtk_dialog_new();

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	gtk_box_set_spacing(
		GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		18
	);

	gtk_dialog_add_buttons(
		GTK_DIALOG (dialog),
		_("_Close"), GTK_RESPONSE_CANCEL,
		NULL
	);

#endif	// GTK(3,12,0)

	if(save_button)
	{
		gtk_widget_set_tooltip_markup(save_button,_("Save transfer information to file"));
		g_signal_connect(save_button,"clicked",G_CALLBACK(save_activity_clicked),activity);
	}

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),worker,TRUE,TRUE,2);

	// Start transfer
	// g_signal_connect(worker,"failed",G_CALLBACK(transfer_failed), dialog);
	g_signal_connect(worker,"success",G_CALLBACK(transfer_success), dialog);
	v3270_ft_worker_start(worker);

	gtk_widget_show_all(dialog);
	int rc = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	return rc;
}

