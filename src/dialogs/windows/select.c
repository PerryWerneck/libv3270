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
#include <gdk/gdkwin32.h>
#include <lib3270/log.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

struct FileSelector
{
	OPENFILENAME 	ofn;
	char		  	szName[260];	///< @brief buffer for file name.
	int				mode;
	BOOL			ok;
};

/*--[ Implement ]------------------------------------------------------------------------------------*/

/*
static gpointer win32_select_file(struct FileSelector *fl)
{
	debug("%s: start",__FUNCTION__);
	if(fl->mode == 1)
	{
		fl->ok = GetSaveFileName(&fl->ofn);
	}
	else
	{
		fl->ok = GetOpenFileName(&fl->ofn);
	}
	debug("%s: end",__FUNCTION__);

	fl->mode = 3;

	return 0;
}


gchar * v3270_select_file(GtkWidget *widget, const gchar *title, const gchar *button, GtkFileChooserAction action, const gchar *filename, const gchar *filter, ...)
{
	gchar *rc = NULL;

	// Get parent windows
	GdkWindow * win = gtk_widget_get_window(gtk_widget_get_toplevel(widget));

	// Setup win32 file selection parameters
	struct FileSelector fl;

	memset(&fl, 0, sizeof(fl));
	fl.ofn.lStructSize		= sizeof(fl.ofn);
	fl.ofn.hwndOwner		= GDK_WINDOW_HWND(win);
	fl.ofn.lpstrFile		= fl.szName;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	fl.ofn.lpstrFile[0] 	= '\0';

	fl.ofn.lpstrFilter		= "All\0*.*\0Text\0*.TXT\0";
	fl.ofn.nFilterIndex		= 1;
	fl.ofn.lpstrInitialDir	= NULL;

	fl.ofn.nMaxFileTitle	= strlen(title);
	fl.ofn.lpstrFileTitle	= (char *) title;

	// Guarda o valor atual
	fl.ofn.nMaxFile = sizeof(fl.szName);
	strncpy(fl.szName, filename, fl.ofn.nMaxFile-1);

	// Obtêm o tipo de diálogo.
	switch(action) {
	case GTK_FILE_CHOOSER_ACTION_OPEN:				// Indicates open mode. The file chooser will only let the user pick an existing file.
		// Enviar arquivo
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646928(v=vs.85).aspx
		fl.ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		fl.mode = 0;
		break;

	case GTK_FILE_CHOOSER_ACTION_SAVE:				// Indicates save mode. The file chooser will let the user pick an existing file, or type in a new filename.
		// Receber arquivo
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646829(v=vs.85).aspx#open_file
		fl.ofn.Flags = OFN_OVERWRITEPROMPT;

		fl.mode = 1;
		break;

	case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:		// Indicates an Open mode for selecting folders. The file chooser will let the user pick an existing folder.
		fl.mode = 3;
		break;

	case GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER:		//Indicates a mode for creating a new folder. The file chooser will let the user name an existing or new folder.
		fl.mode = 3;
		break;

	default:
		return NULL;

	}

	GThread * thd = g_thread_new("SelectFileDialog",(GThreadFunc) win32_select_file, &fl);

	gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
	debug("%s: Keeping main loop alive",__FUNCTION__);
	while(fl.mode != 3) {
		gtk_main_iteration();
	}
	gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);

	debug("%s: Waiting for service thread", __FUNCTION__);
	g_thread_join(thd);
	debug("%s: Service thread ends", __FUNCTION__);

	return rc;
}
*/

gchar * v3270_select_file(GtkWidget *widget, const gchar *title, const gchar *button, GtkFileChooserAction action, const gchar *filename, const gchar *filter, ...)
{
	gchar *rc = NULL;

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
	/*
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
	*/

	if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		rc = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	}

	gtk_widget_destroy(chooser);

	return rc;

}

