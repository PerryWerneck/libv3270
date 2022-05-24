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

 /**
  * @brief Usefull extensions for GtkEntry.
  *
  */

 #include <internals.h>
 #include <v3270/tools.h>
 #include <string.h>

 struct FileEntry {
 	GtkFileChooserAction	  action;
	const gchar				* title;
	const gchar				* pattern;
	const gchar				* name;
	const gchar				* accept;
 };

 static void icon_response(GtkDialog *dialog, int response_id, GtkEntry *entry) {

	if(response_id == GTK_RESPONSE_ACCEPT) {

		g_autofree gchar * filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(filename) {

			// https://github.com/PerryWerneck/pw3270/issues/9#issuecomment-708963229
			//
			// Works here (openSUSE Tumbleweed), BUT: the "open session" dialog comes with a filter
			// set to "TN3270 files" and that's good, but when you save a session pw3270 does NOT add
			// the extension, so how do you know what file extension those "TN3270 files" are supposed to have?
			//
			// I tried .tn3270 but it did not work, so I had to look hard in the internet to find out this page
			// ...
			//
			if(strchr(filename,'.')) {

				// Filename has extension, use it.
				gtk_entry_set_text(entry,filename);

			} else {

				// No extension, add it from the standard.
				const gchar *ext = (const gchar *) g_object_get_data(G_OBJECT(dialog),"default-file-extension");
				if(ext) {
					g_autofree gchar * new_name = g_strconcat(filename,ext,NULL);
					gtk_entry_set_text(entry,new_name);
				} else {
					g_warning("No default file extension set");
					gtk_entry_set_text(entry,filename);
				}

			}

		} else {
			gtk_entry_set_text(entry,"");
		}
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));

 }

 static void icon_press(GtkWidget *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, const struct FileEntry *descr) {

	GtkWidget * dialog =
					gtk_file_chooser_dialog_new(
						gettext(descr->title),
						GTK_WINDOW(gtk_widget_get_toplevel(entry)),
						descr->action,
						_("Cancel"),    GTK_RESPONSE_CANCEL,
						descr->accept,	GTK_RESPONSE_ACCEPT,
						NULL
					);

	if(descr->pattern) {

		GtkFileFilter *filter;

		// Standard filter
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern (filter, descr->pattern);
		gtk_file_filter_set_name(filter, gettext(descr->name));
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);

		const gchar *ext = strchr(descr->pattern,'.');
		if(ext) {
			g_object_set_data_full(G_OBJECT(dialog),"default-file-extension",g_strdup(ext),g_free);
		}

		// All files
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern (filter, "*.*");
		gtk_file_filter_set_name(filter, _("All files"));
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);

	}

	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	const gchar *filename = gtk_entry_get_text(GTK_ENTRY(entry));

	if(filename && *filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),filename);

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(icon_response),entry);

	gtk_widget_show_all(dialog);


 }

 LIB3270_EXPORT GtkWidget * gtk_entry_bind_to_filechooser(GtkWidget *widget, GtkFileChooserAction action, const gchar *title, const gchar *icon_name, const gchar *pattern, const gchar *name) {

	gtk_entry_set_icon_from_icon_name(
		GTK_ENTRY(widget),
		GTK_ENTRY_ICON_SECONDARY,
		icon_name ? icon_name : "document-open"
	);

	// Store data
	gsize szEntry = sizeof(struct FileEntry) + strlen(title) + 4;

	if(pattern) {
		szEntry += (strlen(pattern) + strlen(name));
	}
	struct FileEntry * entry = (struct FileEntry *) g_malloc0(szEntry);
	gtk_widget_bind_ptr(widget,entry);

	entry->action	= action;

	switch(action) {
	case GTK_FILE_CHOOSER_ACTION_OPEN:
		entry->accept = _("_Open");
		break;

	case GTK_FILE_CHOOSER_ACTION_SAVE:
		entry->accept = _("_Save");
		break;

	default:
		entry->accept = _("_Continue");

	}

	entry->title = (const char *) (entry+1);
	strcpy((char *) entry->title,title);

	if(pattern) {
		entry->pattern	= entry->title + strlen(entry->title) +1;
		strcpy((char *) entry->pattern,pattern);

		entry->name 	= entry->pattern + strlen(entry->pattern) + 1;
		strcpy((char *) entry->name,name);
	}

	g_signal_connect(widget,"icon_press",G_CALLBACK(icon_press),(gpointer) entry);

 }
