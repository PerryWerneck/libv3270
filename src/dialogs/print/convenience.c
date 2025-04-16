/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
 #include <v3270/dialogs.h>
 #include <terminal.h>
 #include <lib3270/selection.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 int v3270_print_dialog(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, GError **error)
 {
 	int rc = 0;

 	if(!(widget && GTK_IS_V3270(widget)))
		return errno = EINVAL;

	lib3270_write_event_trace(v3270_get_session(widget),"print action activated (type=%d)\n",(int) mode);

	if(!v3270_is_connected(widget))
		return errno = ENOTCONN;

	// Print operation.
	GtkPrintOperation * operation = v3270_print_operation_new(widget, mode);
	if(!operation)
		return errno = EPERM;

	gtk_print_operation_set_show_progress(operation,TRUE);
	gtk_print_operation_set_allow_async(operation,TRUE);

	GtkPrintOperationResult result = GTK_PRINT_OPERATION_RESULT_ERROR;

	if(error)
	{
		result =
			gtk_print_operation_run(
				operation,
				GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				error
			);

		if(*error)
		{
			rc = (*error)->code ? (*error)->code : -1;
			g_warning("Print operation has failed with errror\"%s\" (rc=%d)",(*error)->message,rc);

		}
		else
		{
			rc = 0;
		}


	}
	else
	{
		GError *err = NULL;

		result =
			gtk_print_operation_run(
				operation,
				GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				&err
			);

		if(err)
		{
			v3270_popup_gerror(
				widget,
				&err,
				NULL,
				_("Print operation has failed")
			);

			rc = -1;
		}
	}

	switch(result)
	{
	case GTK_PRINT_OPERATION_RESULT_ERROR:
		debug("%s: Error on print operation\n",__FUNCTION__);
		g_warning("Error on print operation");
		if(!rc)
			rc = -1;
		break;

	case GTK_PRINT_OPERATION_RESULT_APPLY:
		debug("%s: The print settings should be stored.",__FUNCTION__);
		rc = 0;
		break;

	case GTK_PRINT_OPERATION_RESULT_CANCEL:
		debug("%s: The print operation has been canceled, the print settings should not be stored.", __FUNCTION__);
		break;

	case GTK_PRINT_OPERATION_RESULT_IN_PROGRESS:
		debug("%s: The print operation is running",__FUNCTION__);
		break;

	default:
		g_warning("Unexpected status %d in print operation",(int) result);

	}

	debug("%s(%p)",__FUNCTION__,operation);
	g_object_unref(operation);
	debug("%s(%p)",__FUNCTION__,operation);

	return rc;

 }

 int v3270_print(GtkWidget *widget, GError **error)
 {
	return v3270_print_dialog(widget,(lib3270_get_has_selection(GTK_V3270(widget)->host) ? LIB3270_CONTENT_SELECTED : LIB3270_CONTENT_ALL),error);
 }


 int v3270_print_all(GtkWidget *widget, GError **error)
 {
	return v3270_print_dialog(widget,LIB3270_CONTENT_ALL,error);
 }

 int v3270_print_selected(GtkWidget *widget, GError **error)
 {
	return v3270_print_dialog(widget,LIB3270_CONTENT_SELECTED,error);
 }

 int v3270_print_copy(GtkWidget *widget, GError **error)
 {
	return v3270_print_dialog(widget,LIB3270_CONTENT_COPY,error);
 }

