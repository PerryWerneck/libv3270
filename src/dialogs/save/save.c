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

 static const struct _charsets
 {
	const gchar *name;
	const gchar *description;
 } charsets[] =
 {
	// http://en.wikipedia.org/wiki/Character_encoding
	{ "UTF-8",		N_( "UTF-8"	)								},
	{ "ISO-8859-1", N_( "Western Europe (ISO 8859-1)" ) 		},
	{ "CP1252",		N_( "Windows Western languages (CP1252)" )	},
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

/*
 static void V3270SaveDialog_finalize(V3270SaveDialog *object)
 {
	V3270SaveDialog *dialog = V3270_SAVE_DIALOG(object);
 }
*/

 static void V3270SaveDialog_class_init(V3270SaveDialogClass G_GNUC_UNUSED(*klass))
 {

	debug("%s",__FUNCTION__);
//	G_OBJECT_CLASS(klass)->finalize = V3270SaveDialog_finalize;

 }

 static void cancel_operation(GtkButton G_GNUC_UNUSED(*button), GtkDialog *dialog)
 {
	gtk_dialog_response(dialog,GTK_RESPONSE_CANCEL);
 }

 static void apply_operation(GtkButton G_GNUC_UNUSED(*button), GtkDialog *dialog)
 {
	gtk_dialog_response(dialog,GTK_RESPONSE_APPLY);
 }

#ifdef WIN32
static void select_local_file(GtkButton G_GNUC_UNUSED(*button), v3270ft *dialog) {
#else
static void icon_press(G_GNUC_UNUSED GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, V3270SaveDialog *dialog) {
#endif // WIN32

	gint format = gtk_combo_box_get_active(GTK_COMBO_BOX(dialog->format));
	g_autofree gchar * extension = g_strconcat("*",formats[format].extension,NULL);

	g_autofree gchar *filename =
					v3270_select_file(
						GTK_WIDGET(dialog),
						_("Select file to save"),
						_("Select"),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						gtk_entry_get_text(GTK_ENTRY(dialog->filename)),
						gettext(formats[format].name), extension,
						NULL
					);

	if(filename)
		gtk_entry_set_text(GTK_ENTRY(dialog->filename),filename);

 }

 static void V3270SaveDialog_init(V3270SaveDialog *dialog)
 {
 	//     0--------1---------------------2-------3------------------
 	// 0 - Filename xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.
 	// 1 - Charset  xxxxxxxxx.xxxxxxxxx.  Format: xxxxxxxxx.xxxxxxxxx.


	dialog->mode = LIB3270_CONTENT_ALL;

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// Setup visual elements
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	GtkWidget *widget;
	GtkWidget *button;

	GtkBox * box = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(box),18);

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

#ifdef WIN32
		widget = gtk_button_new_from_icon_name("document-open",GTK_ICON_SIZE_BUTTON);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(select_local_file),dialog);
		gtk_grid_attach(grid,widget,6,0,1,1);
#else
		gtk_entry_set_icon_from_icon_name(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,"document-open");
		gtk_entry_set_icon_activatable(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,TRUE);
		gtk_entry_set_icon_tooltip_text(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,_("Select file"));
		g_signal_connect(G_OBJECT(dialog->filename),"icon-press",G_CALLBACK(icon_press),dialog);
#endif // WIN32

		gtk_entry_set_width_chars(GTK_ENTRY(dialog->filename),60);
		gtk_entry_set_max_length(GTK_ENTRY(dialog->filename),PATH_MAX);
		gtk_grid_attach(grid,GTK_WIDGET(dialog->filename),1,0,3,1);
	}

	// Charset drop-down
	{
		size_t ix;
		const gchar	* scharset	= NULL;

		widget = gtk_label_new_with_mnemonic (_("C_haracter Coding"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,0,1,1,1);

		dialog->charset =  gtk_combo_box_text_new();

		g_get_charset(&scharset);

		g_autofree gchar * text = g_strdup_printf(_("Current (%s)"),scharset);
		gtk_combo_box_text_insert(
			GTK_COMBO_BOX_TEXT(dialog->charset),
			0,
			scharset,
			text
		);

		gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->charset),0);

		gtk_grid_attach(grid,dialog->charset,1,1,1,1);

		for(ix=0;ix<G_N_ELEMENTS(charsets);ix++)
		{
			if(g_ascii_strcasecmp(charsets[ix].name,scharset))
			{
				gtk_combo_box_text_insert(
					GTK_COMBO_BOX_TEXT(dialog->charset),
					ix+1,
					charsets[ix].name,
					gettext(charsets[ix].description)
				);
			}
		}

	}

	// Format drop-down
	{
		size_t ix;

		widget = gtk_label_new_with_mnemonic (_("File _Format"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,2,1,1,1);

		dialog->format = gtk_combo_box_text_new();
		gtk_grid_attach(grid,dialog->format,3,1,1,1);

		for(ix=0;ix<G_N_ELEMENTS(formats);ix++)
		{
			gtk_combo_box_text_insert(
				GTK_COMBO_BOX_TEXT(dialog->format),
				ix,
				formats[ix].extension,
				gettext(formats[ix].name)
			);
		}

		gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->format),0);

	}


	// Buttons
	// https://developer.gnome.org/icon-naming-spec/
#if GTK_CHECK_VERSION(3,14,0)
	widget = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
#else
	widget = NULL;
#endif // GTK(3,14,0)

	if(widget)
	{
		// Have header bar
		button = gtk_button_new_with_mnemonic(_("_Cancel"));
		gtk_widget_set_tooltip_markup(button,_("Click to cancel operation"));
		gtk_header_bar_pack_start(GTK_HEADER_BAR(widget),button);
		g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(cancel_operation),dialog);

		button = gtk_button_new_with_mnemonic(_("_Save"));
		gtk_widget_set_tooltip_markup(button,_("Click to save file"));
		gtk_header_bar_pack_end(GTK_HEADER_BAR(widget),button);
		g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(apply_operation),dialog);
	}
	else
	{
		gtk_dialog_add_buttons(
			GTK_DIALOG (dialog),
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Save"), GTK_RESPONSE_APPLY,
			NULL
		);
	}

 }

 GtkWidget * v3270_save_dialog_new(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, const gchar *filename)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

 	static const gchar * titles[] =
 	{
 		N_("Save terminal contents"),
 		N_("Save selected area"),
 		N_("Save copied data"),
 	};

	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	// Create dialog
	V3270SaveDialog * dialog = V3270_SAVE_DIALOG(
									g_object_new(
										GTK_TYPE_V3270SaveDialog,
										"use-header-bar", (use_header ? 1 : 0),
										NULL)
									);

	dialog->mode		= mode;
	dialog->terminal	= widget;

	if( (size_t) mode < G_N_ELEMENTS(titles))
	{
		gtk_window_set_title(GTK_WINDOW(dialog),gettext(titles[(size_t) mode]));
	}

	if(filename)
		gtk_entry_set_text(GTK_ENTRY(dialog->filename),filename);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	return GTK_WIDGET(dialog);
 }

 void v3270_save_dialog_apply(GtkWidget *widget, GError **error)
 {
 	V3270SaveDialog * dialog = V3270_SAVE_DIALOG(widget);

 	if(!v3270_is_connected(dialog->terminal))
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENOTCONN));
		return;
	}

	// Get selection
	GList 		* dynamic	= NULL;
	const GList * selection = NULL;

	switch(dialog->mode)
	{
	case LIB3270_CONTENT_ALL:
		break;

	case LIB3270_CONTENT_COPY:
		selection = v3270_get_selection_blocks(dialog->terminal);
		break;

	case LIB3270_CONTENT_SELECTED:
		dynamic = g_new0(GList,1);
		dynamic->data = (gpointer) lib3270_get_selection(v3270_get_session(dialog->terminal),0);
		selection = dynamic;
		break;

	default:
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,_( "Unexpected mode %d" ),(int) dialog->mode);
		return;
	}

	if(!selection)
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENODATA));
	}
	else
	{
		const gchar			* encoding	= gtk_combo_box_get_active_id(GTK_COMBO_BOX(dialog->charset));
		g_autofree gchar 	* text		= NULL;

		debug("Encoding: %s",encoding);

		switch(gtk_combo_box_get_active(GTK_COMBO_BOX(dialog->format)))
		{
		case 0: // "Plain text"
			text = v3270_get_selection_as_text(GTK_V3270(dialog->terminal), selection, encoding);
			break;

		case 1: // "Comma-separated values (CSV)"
			text = v3270_get_selection_as_table(GTK_V3270(dialog->terminal),selection,";",encoding);
			break;

		case 2: // "HyperText Markup Language (HTML)"
			text = v3270_get_selection_as_html_div(GTK_V3270(dialog->terminal),selection,encoding);
			break;

		default:
			*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,_( "Unexpected format %d" ),(int) gtk_combo_box_get_active(GTK_COMBO_BOX(dialog->format)));
		}

		if(text)
		{
			debug("%s",text);

			g_file_set_contents(
				gtk_entry_get_text(GTK_ENTRY(dialog->filename)),
				text,
				-1,
				error
			);

		}

	}

	if(dynamic)
		g_list_free_full(dynamic,(GDestroyNotify) lib3270_free);

 }

 void v3270_save_dialog_run(GtkWidget *widget)
 {
	if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_APPLY)
	{
		GError * error 	= NULL;
		v3270_save_dialog_apply(widget,&error);

		if(error)
		{
			GtkWidget *popup = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
				_("Can't save %s"),gtk_entry_get_text(GTK_ENTRY(V3270_SAVE_DIALOG(widget)->filename))
			);

			gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
			g_error_free(error);

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);

		}

	}

 }


