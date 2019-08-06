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
 #include <errno.h>
 #include <string.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT int	v3270_save(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, const gchar *filename, GError **error)
 {
	g_autofree gchar * message = NULL;

	if(filename)
	{
		message = g_strdup_printf("Can't save contents to file \"%s\"",filename);
	}
	else
	{
		message = g_strdup("Can't save contents to file");
	}

 	if(!v3270_is_connected(widget))
	{
		if(error)
		{
			*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENOTCONN));
		}
		else
		{
			GtkWidget *popup = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
				"%s", message
			);

			gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",strerror(ENOTCONN));

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);
		}
		return errno = ENOTCONN;
	}

	int rc = lib3270_save(v3270_get_session(widget),mode,filename);

	if(!rc)
		return 0;

	if(error)
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),rc,"%s",strerror(rc));
	}
	else
	{
		GtkWidget *popup = gtk_message_dialog_new_with_markup(
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
			"%s", message
		);

		gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",strerror(ENOTCONN));

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);
	}

	return errno = rc;
 }

 int v3270_save_all(GtkWidget *widget, const gchar *filename, GError **error)
 {
	return v3270_save(widget,LIB3270_CONTENT_ALL,filename,error);
 }

 int v3270_save_selected(GtkWidget *widget, const gchar *filename, GError **error)
 {
	return v3270_save(widget,LIB3270_CONTENT_SELECTED,filename,error);
 }

 int v3270_save_copy(GtkWidget *widget, const gchar *filename, GError **error)
 {
	return v3270_save(widget,LIB3270_CONTENT_COPY,filename,error);
 }

