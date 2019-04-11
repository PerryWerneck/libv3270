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
 * Este programa está nomeado como - possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include "../private.h"
#include <stdarg.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

gchar * v3270_select_file(GtkWidget *widget, const gchar *title, const gchar *button, GtkFileChooserAction action, const gchar *filename, const gchar *filter, ...)
{
	gchar *rc = NULL;

#if GTK_CHECK_VERSION(3,20,0)

	GtkFileChooserNative *native =
		gtk_file_chooser_native_new
		(
			title,
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			action,
			button,
			_( "_Cancel" )
		);


	// Setup filename
	if(filename && *filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(native),filename);

	// Setup filters
	va_list args;
	va_start (args, filter);
	while(filter)
	{
		const gchar * name = va_arg(args, const gchar *);
		if(!name)
			break;

		const gchar * pattern = va_arg(args, const gchar *);
		if(!pattern)
			break;

		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter,name);
		gtk_file_filter_add_pattern(filter, pattern);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

	}
	va_end(args);

	// Run dialog
	if(gtk_native_dialog_run(GTK_NATIVE_DIALOG (native)) == GTK_RESPONSE_ACCEPT) {
		rc = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native));
	}

	g_object_unref(native);

#else

	GtkWidget * chooser =
		gtk_file_chooser_dialog_new
		(
			title,
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			action,
			_("_Cancel" ),	GTK_RESPONSE_CANCEL,
			button, GTK_RESPONSE_ACCEPT,
			NULL
		);

	if(filename && *filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser),filename);

	// Setup filters
	va_list args;
	va_start (args, filter);
	while(filter)
	{
		const gchar * name = va_arg(args, const gchar *);
		if(!name)
			break;

		const gchar * pattern = va_arg(args, const gchar *);
		if(!pattern)
			break;

		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter,name);
		gtk_file_filter_add_pattern(filter, pattern);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);

	}
	va_end(args);

	if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		rc = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	}

	gtk_widget_destroy(chooser);


#endif // GTK 3.20

	return rc;

}

