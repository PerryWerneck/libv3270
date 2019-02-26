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

}

static void transfer_failed(GtkWidget *worker, const gchar *msg, const gchar *description, GtkDialog *dialog)
{
	debug("%s(\"%s\",\"%s\")",__FUNCTION__,msg,description);

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

	// Create dialog
#if GTK_CHECK_VERSION(3,12,0)

	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

		gtk_window_set_title(GTK_WINDOW(dialog),(options & LIB3270_FT_OPTION_RECEIVE) ? _( "Receiving file" ) : _( "Sending file" ));

	if(use_header)
	{
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(gtk_dialog_get_header_bar(dialog)),_("Starting transfer"));
		g_signal_connect(worker,"ft-state-changed",G_CALLBACK(header_on_state_changed), dialog);
	}

#else

	GtkWidget * dialog = gtk_dialog_new_with_buttons(
		(options & LIB3270_FT_OPTION_RECEIVE) ? _( "Receiving file" ) : _( "Sending file" ),
		GTK_WINDOW(gtk_widget_get_toplevel(widget)),
		GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
		_( "_Close" ), GTK_RESPONSE_CANCEL,
		NULL
	);

#endif	// GTK(3,12,0)

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),18);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),worker,TRUE,TRUE,2);

	// Start transfer
	g_signal_connect(worker,"failed",G_CALLBACK(transfer_failed), dialog);
	g_signal_connect(worker,"success",G_CALLBACK(transfer_success), dialog);
	v3270_ft_worker_start(worker);

	gtk_widget_show_all(dialog);
	int rc = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	return rc;
}

