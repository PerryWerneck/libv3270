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

/*--[ GTK Requires ]---------------------------------------------------------------------------------*/

 G_DEFINE_TYPE(V3270SaveDialog, V3270SaveDialog, GTK_TYPE_DIALOG);

/*--[ Formats ]--------------------------------------------------------------------------------------*/

 static const struct _formats
 {
	const gchar *name;
	const gchar *extension;
 } formats[] =
 {
	{
		N_("Plain text"),
		".txt"
	},
	{
		N_("Comma-separated values (CSV)"),
		".csv"
	},
	{
		N_("HyperText Markup Language (HTML)"),
		".html"
	}
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270SaveDialog_class_init(V3270SaveDialogClass G_GNUC_UNUSED(*klass))
 {

	debug("%s",__FUNCTION__);
//	G_OBJECT_CLASS(klass)->finalize = V3270SaveDialog_finalize;

 }

#ifdef WIN32
static void icon_press(GtkEntry G_GNUC_UNUSED(*entry), G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, V3270SaveDialog *widget)
{
	g_autofree gchar *filename =
						v3270_select_file(
								GTK_WIDGET(widget),
								_( "Select destination file"),
								_("Select"),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								gtk_entry_get_text(GTK_ENTRY(widget->filename))
						);

	if(filename && *filename) {
		gtk_entry_set_text(GTK_ENTRY(widget->filename),filename);
	}

}
#else
static void icon_press(GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, V3270SaveDialog *widget)
{

	GtkWidget * dialog =
		gtk_file_chooser_dialog_new(
				_( "Select destination file"),
				GTK_WINDOW(widget),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				_("Cancel"),	GTK_RESPONSE_CANCEL,
				_("Select"),	GTK_RESPONSE_ACCEPT,
				NULL );


	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
	g_signal_connect(G_OBJECT(dialog),"close",G_CALLBACK(v3270_dialog_close),NULL);

	const gchar *filename = gtk_entry_get_text(entry);

	if(filename && *filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),filename);
	else
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), _("Untitled document"));

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		gtk_entry_set_text(entry,gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

	gtk_widget_destroy(dialog);

 }
#endif // _WIN32


 static void filename_changed(GtkEntry *entry, V3270SaveDialog *dialog)
 {
 	const gchar * text = gtk_entry_get_text(entry);
	GtkWidget * button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),GTK_RESPONSE_APPLY);

 	if(!(text && *text)) {
		gtk_widget_set_sensitive(button,FALSE);
		return;
 	}

 	if(g_str_has_suffix(text,G_DIR_SEPARATOR_S)) {
		gtk_widget_set_sensitive(button,FALSE);
		return;
 	}

 	g_autofree gchar * dirname = g_path_get_dirname(text);
 	g_autofree gchar * basename = g_path_get_basename(text);

 	gtk_widget_set_sensitive(button,g_file_test(dirname,G_FILE_TEST_IS_DIR) && *basename && (*basename != '.'));

 	debug("*************[%s]***********",basename);

	const gchar * extension = strrchr(basename,'.');
	if(!extension)
		return;

	extension++;
	const gchar	* format = gtk_combo_box_get_active_id(dialog->format);
	if(*format == '.')
		format++;

	if(g_ascii_strcasecmp(extension,format) == 0)
		return;

	gint column = gtk_combo_box_get_id_column(dialog->format);
	GtkTreeModel * model = gtk_combo_box_get_model(dialog->format);
	GtkTreeIter iter;

	debug("id_column=%d",column);

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			g_autofree gchar *id = NULL;
			gtk_tree_model_get(model, &iter, column, &id, -1);

			if(g_ascii_strcasecmp(extension,id + (*id == '.' ? 1 : 0)) == 0)
			{
				gtk_combo_box_set_active_iter(dialog->format,&iter);
				break;
			}

		} while(gtk_tree_model_iter_next(model,&iter));
	}

 }

 static void fileformat_changed(GtkComboBox *formats, GtkEntry *entry)
 {
	const gchar * text = gtk_entry_get_text(entry);

 	if(!(text && *text))
		return;

	gchar * extension = strrchr(text,'.');
	if(!extension)
		return;

	extension++;
	const gchar	* format = gtk_combo_box_get_active_id(formats);
	if(*format == '.')
		format++;

	if(g_ascii_strcasecmp(extension,format) == 0)
		return;

	size_t szFilename = strlen(text) + strlen(format);
	g_autofree gchar * filename = g_malloc0(szFilename + 1);

	strncpy(filename,text,szFilename);
	extension = strrchr(filename,'.');
	if(extension)
	{
		*(++extension) = 0;
		strncat(filename, format + (*format == '.' ? 1 : 0),szFilename);
		gtk_entry_set_text(entry,filename);
	}

 }

 static void V3270SaveDialog_init(V3270SaveDialog *dialog)
 {
 	//     0--------1---------------------2-------3--------------------4
 	// 0 - Filename xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx. x
 	// 1 - Charset  xxxxxxxxx.xxxxxxxxx.  Format: xxxxxxxxx.xxxxxxxxx.


	dialog->mode = LIB3270_CONTENT_ALL;

	// gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// Setup visual elements
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	GtkWidget *widget;

	GtkBox * box = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);

#ifdef G_OS_UNIX
	gtk_container_set_border_width(GTK_CONTAINER(box),18);
#endif // LINUX

	GtkGrid * grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),12);
	gtk_box_pack_start(box,GTK_WIDGET(grid),TRUE,TRUE,2);

	// Filename entry
	{
		dialog->filename = gtk_entry_new();
		gtk_widget_set_hexpand(GTK_WIDGET(dialog->filename),TRUE);

		widget = gtk_label_new_with_mnemonic(_("_Filename"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,0,0,1,1);
		gtk_label_set_mnemonic_widget(GTK_LABEL(widget),dialog->filename);

		gtk_entry_set_icon_from_icon_name(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,"document-save-as");
		gtk_entry_set_icon_activatable(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,TRUE);
		gtk_entry_set_icon_tooltip_text(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,_("Select file"));
		g_signal_connect(G_OBJECT(dialog->filename),"icon-press",G_CALLBACK(icon_press),dialog);

		gtk_entry_set_width_chars(GTK_ENTRY(dialog->filename),60);
		gtk_entry_set_max_length(GTK_ENTRY(dialog->filename),PATH_MAX);
		gtk_grid_attach(grid,GTK_WIDGET(dialog->filename),1,0,3,1);
	}

	// Charset drop-down
	{
		widget = gtk_label_new_with_mnemonic (_("C_haracter Coding"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,0,1,1,1);

		dialog->charset = v3270_charset_combo_box_new();
		gtk_grid_attach(grid,dialog->charset,1,1,1,1);

	}

	// Format drop-down
	{
		size_t ix;

		widget = gtk_label_new_with_mnemonic (_("File _Format"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,2,1,1,1);

		dialog->format = GTK_COMBO_BOX(gtk_combo_box_text_new());

		gtk_grid_attach(grid,GTK_WIDGET(dialog->format),3,1,1,1);

		for(ix=0;ix<G_N_ELEMENTS(formats);ix++)
		{
			gtk_combo_box_text_insert(
				GTK_COMBO_BOX_TEXT(dialog->format),
				-1,
				formats[ix].extension,
				g_dgettext(GETTEXT_PACKAGE,formats[ix].name)
			);
		}

		gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->format),0);

		// Image formats.
		GSList *img_formats = gdk_pixbuf_get_formats();
		GSList *img_format;

		for(img_format = img_formats;img_format;img_format = g_slist_next(img_format))
		{
			GdkPixbufFormat * pixFormat = (GdkPixbufFormat *) img_format->data;

			if (gdk_pixbuf_format_is_writable(pixFormat))
			{
				gtk_combo_box_text_insert(
					GTK_COMBO_BOX_TEXT(dialog->format),
					-1,
					gdk_pixbuf_format_get_name(pixFormat),
					gdk_pixbuf_format_get_description(pixFormat)
				);
			}

		}

		g_signal_connect(dialog->filename,"changed",G_CALLBACK(filename_changed),dialog);
		g_signal_connect(dialog->format,"changed",G_CALLBACK(fileformat_changed),dialog->filename);

	}


	// Buttons
	// https://developer.gnome.org/icon-naming-spec/
	gtk_dialog_add_buttons(
		GTK_DIALOG (dialog),
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Save"), GTK_RESPONSE_APPLY,
		NULL
	);

	if(!v3270_dialog_get_use_header()) {
		GtkWidget * content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
		gtk_box_set_spacing(GTK_BOX(content_area),3);
	}

	gtk_widget_set_sensitive(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),GTK_RESPONSE_APPLY),FALSE);

 }

 GtkWidget * v3270_save_dialog_new(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, const gchar *filename)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

 	static const gchar * titles[] =
 	{
 		N_("Save terminal contents"),
 		N_("Save selected area"),
 		N_("Save copy"),
 	};

	// Create dialog
	V3270SaveDialog * dialog = V3270_SAVE_DIALOG(
									g_object_new(
										GTK_TYPE_V3270SaveDialog,
										"use-header-bar", v3270_dialog_get_use_header() ? 1 : 0,
										NULL)
									);

	dialog->mode		= mode;
	dialog->terminal	= widget;

	if( (size_t) mode < G_N_ELEMENTS(titles))
	{
		gtk_window_set_title(GTK_WINDOW(dialog),g_dgettext(GETTEXT_PACKAGE,titles[(size_t) mode]));
	}

	if(filename)
		gtk_entry_set_text(GTK_ENTRY(dialog->filename),filename);

#ifdef G_OS_UNIX
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
#endif // G_OS_UNIX

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	return GTK_WIDGET(dialog);
 }

 static const gchar * get_filename(V3270SaveDialog * dialog)
 {
	const gchar * filename	= gtk_entry_get_text(GTK_ENTRY(dialog->filename));
	gint		  response 	= GTK_RESPONSE_OK;

	if(g_file_test(filename,G_FILE_TEST_EXISTS))
	{
		GtkWidget * confirmation =
						gtk_message_dialog_new_with_markup(
								GTK_WINDOW(dialog),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,
								_("The file \"%s\" already exists. Replace it?"),
								filename
							);

		response = gtk_dialog_run(GTK_DIALOG(confirmation));
		gtk_widget_destroy(confirmation);
	}

	return (response == GTK_RESPONSE_OK ? filename : NULL);

 }

 static void save_as_text(V3270SaveDialog * dialog, size_t index, GError **error)
 {
	// Get selection
	GList 		* dynamic	= NULL;
	const GList * selection = NULL;

	debug("%s(%d)",__FUNCTION__,dialog->mode);

	switch(dialog->mode)
	{
	case LIB3270_CONTENT_ALL:
		debug("%s","LIB3270_CONTENT_ALL");
		selection = dynamic = g_list_append_lib3270_selection(dynamic, v3270_get_session(dialog->terminal),TRUE);
		break;

	case LIB3270_CONTENT_COPY:
		debug("%s","LIB3270_CONTENT_COPY");
		selection = v3270_get_selection_blocks(dialog->terminal);
		break;

	case LIB3270_CONTENT_SELECTED:
		debug("%s","LIB3270_CONTENT_SELECTED");
		selection = dynamic = g_list_append_lib3270_selection(dynamic, v3270_get_session(dialog->terminal),FALSE);
		break;

	default:
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,_( "Unexpected mode %d" ),(int) dialog->mode);
		return;
	}

	debug("Selection=%p",selection);

	if(!selection)
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENODATA));
	}
	else
	{
		const gchar			* encoding	= gtk_combo_box_get_active_id(GTK_COMBO_BOX(dialog->charset));
		g_autofree gchar 	* text		= NULL;

		debug("Encoding: %s",encoding);

		switch(index)
		{
		case 0: // "Plain text"
			text = v3270_get_selection_as_text(GTK_V3270(dialog->terminal), selection, encoding, dialog->mode == LIB3270_CONTENT_ALL);
			break;

		case 1: // "Comma-separated values (CSV)"
			text = v3270_get_selection_as_table(GTK_V3270(dialog->terminal),selection,";",encoding, dialog->mode == LIB3270_CONTENT_ALL);
			break;

		case 2: // "HyperText Markup Language (HTML)"
			text = v3270_get_selection_as_html_div(GTK_V3270(dialog->terminal),selection,encoding, dialog->mode == LIB3270_CONTENT_ALL, V3270_SELECTION_DEFAULT);
			break;

		default:
			*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,_( "Unexpected format %d" ),(int) gtk_combo_box_get_active(GTK_COMBO_BOX(dialog->format)));
		}

		if(text)
		{
			const gchar * filename = get_filename(dialog);

			if(filename)
			{
				g_file_set_contents(
					gtk_entry_get_text(GTK_ENTRY(dialog->filename)),
					text,
					-1,
					error
				);
			}

		}

	}

	#pragma GCC diagnostic push
#ifdef _WIN32
	#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif // _WIN32

	if(dynamic)
		g_list_free_full(dynamic,(GDestroyNotify) lib3270_free);

	#pragma GCC diagnostic pop
 }

 static void save_as_image(V3270SaveDialog * dialog, const gchar *format, GError **error)
 {
	debug("%s(%d)",__FUNCTION__,dialog->mode);

	GdkPixbuf * pixbuf = NULL;

	switch(dialog->mode)
	{
	case LIB3270_CONTENT_ALL:
		debug("%s","LIB3270_CONTENT_ALL");
		pixbuf = v3270_get_as_pixbuf(dialog->terminal);
		break;

	case LIB3270_CONTENT_COPY:
		{
			debug("%s","LIB3270_CONTENT_COPY");
			const GList * selection = v3270_get_selection_blocks(dialog->terminal);
			pixbuf = v3270_get_selection_as_pixbuf(GTK_V3270(dialog->terminal), selection, FALSE);
		}
		break;

	case LIB3270_CONTENT_SELECTED:
		{
			debug("%s","LIB3270_CONTENT_SELECTED");
			GList * selection = g_list_append_lib3270_selection(NULL, v3270_get_session(dialog->terminal),FALSE);
			debug("Selection=%p",selection);
			pixbuf = v3270_get_selection_as_pixbuf(GTK_V3270(dialog->terminal), selection, FALSE);
			g_list_free_full(selection,(GDestroyNotify) lib3270_free);
		}
		break;

	default:
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,_( "Unexpected mode %d" ),(int) dialog->mode);
		return;
	}

	debug("pixbuff=%p",pixbuf);

	if(pixbuf)
	{
		const gchar * filename = get_filename(dialog);

		debug("Filename=%p",filename);
		if(filename)
		{
			gdk_pixbuf_save(pixbuf,filename,format,error,NULL);
		}

		g_object_unref(pixbuf);
	}
	else
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),-1,_( "Error saving image" ));
	}

 }

 void v3270_save_dialog_apply(GtkWidget *widget, GError **error)
 {
 	size_t ix;
 	V3270SaveDialog * dialog = V3270_SAVE_DIALOG(widget);

 	if(!v3270_is_connected(dialog->terminal))
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENOTCONN));
		return;
	}

	// Get type ID
	const gchar	* format = gtk_combo_box_get_active_id(GTK_COMBO_BOX(dialog->format));

	// Check for text formats.
	for(ix=0;ix<G_N_ELEMENTS(formats);ix++)
	{
		if(!strcmp(formats[ix].extension,format))
		{
			// Is text format, save it
			save_as_text(dialog, ix, error);
			return;
		}

	}

	save_as_image(dialog, format, error);

 }

 void v3270_save_dialog_run(GtkWidget *widget)
 {
	if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_APPLY)
	{
		GError * error 	= NULL;
		v3270_save_dialog_apply(widget,&error);

		v3270_popup_gerror(
				widget,
				&error,
				NULL,
				_("Can't save %s"),gtk_entry_get_text(GTK_ENTRY(V3270_SAVE_DIALOG(widget)->filename))
		);

	}

 }


