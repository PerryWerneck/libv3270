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
 * Este programa está nomeado como selection.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <clipboard.h>
 #include <lib3270/toggle.h>
 #include <v3270/dialogs.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void text_received(G_GNUC_UNUSED  GtkClipboard *clipboard, const gchar *text, GtkWidget *widget)
{
	v3270_input_text(widget,text,"UTF-8");
}

static gboolean has_target(const GdkAtom atom, const GdkAtom *atoms, const gint n_atoms)
{
	gint ix;

	for(ix = 0; ix < n_atoms; ix++)
	{
		if(atom == atoms[ix])
			return TRUE;
	}

    return FALSE;
}

static void formatted_received(GtkClipboard *clipboard, GtkSelectionData *selection_data, GtkWidget *widget)
{
	const struct SelectionHeader *selection = (const struct SelectionHeader *) gtk_selection_data_get_data(selection_data);

	v3270 * terminal = GTK_V3270(widget);

	debug(
		"Received formatted data with %u bytes: Build=%u rows=%u cols=%u",
			selection->length,
			selection->build,
			selection->rows,
			selection->cols
	);

	if(selection->cols != lib3270_get_width(terminal->host) || selection->rows != lib3270_get_height(terminal->host))
	{
		GtkWidget * dialog =
					gtk_message_dialog_new(
						GTK_WINDOW(gtk_widget_get_toplevel(widget)),
						GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_INFO,
						GTK_BUTTONS_NONE,
						_("Not the same terminal type")
					);


		gtk_window_set_title(GTK_WINDOW(dialog),_("Can't paste"));

		gtk_dialog_add_buttons(
			GTK_DIALOG (dialog),
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Paste as text"), GTK_RESPONSE_APPLY,
			NULL
		);

		gtk_dialog_set_default_response(GTK_DIALOG (dialog),GTK_RESPONSE_APPLY);

		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		gtk_widget_destroy(dialog);

		if(response == GTK_RESPONSE_APPLY)
		{
			gtk_clipboard_request_text(
						clipboard,
						(GtkClipboardTextReceivedFunc) text_received,
						(gpointer) widget
			);
		}

		return;

	}

	if(!v3270_set_from_data_block(terminal, selection))
	{
		debug("%s: Can't paste data",__FUNCTION__);
		if(
			v3270_popup_toggleable_dialog(
				widget,
				V3270_TOGGLEABLE_DIALOG_PASTE_FAILED,
				_("Can't paste"),
				_("Unable to paste formatted data."),
				_("None of the screens in the clipboard match with the current one."),
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("_Paste as text"), GTK_RESPONSE_APPLY,
				NULL
			) == GTK_RESPONSE_APPLY)
		{
			gtk_clipboard_request_text(
				clipboard,
				(GtkClipboardTextReceivedFunc) text_received,
				(gpointer) widget
			);
		}

		/*
		GtkResponseType response = GTK_V3270(terminal)->responses[V3270_TOGGLEABLE_DIALOG_PASTE_FAILED];

		if(response == GTK_RESPONSE_NONE)
		{
			// No predefined response, ask.
			GtkWidget * dialog =
						gtk_message_dialog_new(
							GTK_WINDOW(gtk_widget_get_toplevel(widget)),
							GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_NONE,
							_("Unable to paste formatted data")
						);


			gtk_window_set_title(GTK_WINDOW(dialog),_("Can't paste"));

			gtk_dialog_add_buttons(
				GTK_DIALOG (dialog),
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("_Paste as text"), GTK_RESPONSE_APPLY,
				NULL
			);

			gtk_dialog_set_default_response(GTK_DIALOG (dialog),response);

			gint response = gtk_dialog_run(GTK_DIALOG(dialog));

			gtk_widget_destroy(dialog);
		}

		if(response == GTK_RESPONSE_APPLY)
		{
			gtk_clipboard_request_text(
						clipboard,
						(GtkClipboardTextReceivedFunc) text_received,
						(gpointer) widget
			);
		}

		*/
		return;


	}

}

static void targets_received(GtkClipboard *clipboard, GdkAtom *atoms, gint n_atoms, GtkWidget *widget)
{

	// If smart paste is enabled try to get formatted clipboard.
	debug("%s: Smart paste is %s", __FUNCTION__, (lib3270_get_toggle(GTK_V3270(widget)->host,LIB3270_TOGGLE_SMART_PASTE) ? "enabled" : "disabled"));
	if(lib3270_get_toggle(GTK_V3270(widget)->host,LIB3270_TOGGLE_SMART_PASTE) && has_target(GTK_V3270_GET_CLASS(widget)->clipboard_formatted,atoms,n_atoms))
	{
		debug("Clipboard as TN3270 \"%s\" data",gdk_atom_name(GTK_V3270_GET_CLASS(widget)->clipboard_formatted));

		gtk_clipboard_request_contents(
			clipboard,
			GTK_V3270_GET_CLASS(widget)->clipboard_formatted,
			(GtkClipboardReceivedFunc) formatted_received,
			(gpointer) widget
		);

		return;
	}

	// No smart paste or formatted data on clipboard, request as text.
	gtk_clipboard_request_text(
				clipboard,
				(GtkClipboardTextReceivedFunc) text_received,
				(gpointer) widget
	);

}

LIB3270_EXPORT void v3270_clipboard_get_from_url(GtkWidget *widget, const gchar *url)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,GTK_V3270(widget)->selection.target);

	if(!url || !*url || g_str_has_prefix(url,"clipboard://") || g_str_has_prefix(url,"tn3270://"))
	{
		gtk_clipboard_request_targets(
			clipboard,
			(GtkClipboardTargetsReceivedFunc) targets_received,
			(gpointer) widget
		);
	}
	else if(g_str_has_prefix(url,"text://"))
	{
		gtk_clipboard_request_text(
			clipboard,
			(GtkClipboardTextReceivedFunc) text_received,
			(gpointer) widget
		);

	}
	else if(g_str_has_prefix(url,"file://"))
	{
		GtkWidget * dialog = v3270_load_dialog_new(widget, url+7);
		gtk_widget_show_all(dialog);
		v3270_load_dialog_run(dialog);
		gtk_widget_destroy(dialog);
	}

}


LIB3270_EXPORT void v3270_paste(GtkWidget *widget)
{
	v3270_clipboard_get_from_url(widget,NULL);
}

LIB3270_EXPORT void v3270_paste_text(GtkWidget *widget)
{
	v3270_clipboard_get_from_url(widget,"text://");
}

