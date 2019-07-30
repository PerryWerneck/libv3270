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
 #include <sys/param.h>
 #include <terminal.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 int v3270_print(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, GError **error)
 {
 	int rc;

 	if(!(widget && GTK_IS_V3270(widget)))
	{
		return errno = EINVAL;
	}

	/*
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
				_("Can't print data")
			);

			gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",strerror(ENOTCONN));

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);
		}

		return errno = ENOTCONN;
	}
	*/

	lib3270_trace_event(v3270_get_session(widget),"print action activated (type=%d)",(int) mode);

	// Print operation.
	V3270PrintOperation * operation = v3270_print_operation_new(widget, mode);

	if(error)
	{
		gtk_print_operation_run(
				GTK_PRINT_OPERATION(operation),
				GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				error
		);

		rc = (*error == NULL ? 0 : -1);

	}
	else
	{
		GError *err = NULL;

		gtk_print_operation_run(
				GTK_PRINT_OPERATION(operation),
				GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				&err
		);

		if(err)
		{
			GtkWidget *popup = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
				_("Can't print data")
			);

			gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",strerror(ENOTCONN));

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);

			g_error_free(err);

			rc = -1;
		}
	}

	g_object_unref(operation);

	return rc;

 }

 int v3270_print_all(GtkWidget *widget, GError **error)
 {
	return v3270_print(widget,LIB3270_CONTENT_ALL,error);
 }

 int v3270_print_selected(GtkWidget *widget, GError **error)
 {
	return v3270_print(widget,LIB3270_CONTENT_SELECTED,error);
 }

 int v3270_print_copy(GtkWidget *widget, GError **error)
 {
	return v3270_print(widget,LIB3270_CONTENT_COPY,error);
 }

