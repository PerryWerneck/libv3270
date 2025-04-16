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
 * Este programa está nomeado como - possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <clipboard.h>
 #include <lib3270/selection.h>
 #include <lib3270/toggle.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

void v3270_clipboard_clear(G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED  GObject *obj)
{
	v3270 * terminal = GTK_V3270(obj);

	if(!lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_KEEP_SELECTED))
	{
		v3270_unselect(GTK_WIDGET(obj));
		v3270_clear_selection(terminal);
	}

}

void v3270_clipboard_get(G_GNUC_UNUSED  GtkClipboard *clipboard, GtkSelectionData *selection, guint target, GObject *obj)
{
	v3270 * terminal = GTK_V3270(obj);

	if(!terminal->selection.blocks)
	{
		return;
	}

	switch(target)
	{
	case CLIPBOARD_TYPE_TEXT:   // Get clipboard contents as text
		{
			g_autofree gchar *text = v3270_get_copy_as_text(terminal,"UTF-8");
			gtk_selection_data_set_text(selection,text,-1);
		}
		break;

	case CLIPBOARD_TYPE_CSV:
		{
			g_autofree gchar *text = v3270_get_copy_as_table(terminal,";","UTF-8");
			debug("Selection:\n%s",text);
			gtk_selection_data_set(
				selection,
				gdk_atom_intern_static_string("text/csv"),
				8,
				(guchar *) text,
				strlen(text)
			);
		}
		break;

	case CLIPBOARD_TYPE_HTML:
		{
			g_autofree gchar *text = v3270_get_copy_as_html(terminal,"UTF-8");
			//debug("Selection:\n%s",text);
			gtk_selection_data_set(
				selection,
				gdk_atom_intern_static_string("text/html"),
				8,
				(guchar *) text,
				strlen(text)
			);
		}
		break;

	case CLIPBOARD_TYPE_V3270_FORMATTED:
		{
			g_autofree gchar *data = v3270_get_copy_as_data_block(terminal);
			gtk_selection_data_set(
				selection,
				GTK_V3270_GET_CLASS(obj)->clipboard_formatted,
				8,
				(guchar *) data,
				((struct SelectionHeader *) data)->length
			);
		}
		break;

	case CLIPBOARD_TYPE_PIXBUFF:
		{
			GdkPixbuf * pixbuff = v3270_get_selection_as_pixbuf(terminal, terminal->selection.blocks, FALSE);

			debug("%s: pixbuff=%p (blocks=%p)",__FUNCTION__,pixbuff,terminal->selection.blocks);

			if(pixbuff)
			{
				gtk_selection_data_set_pixbuf(selection,pixbuff);
				g_object_unref(pixbuff);
			}
		}
		break;

	default:
		g_warning("Unexpected clipboard type %d\n",target);
	}
}

void v3270_update_system_clipboard(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

    if(!terminal->selection.blocks)
    {
    	// No clipboard data, return.
    	v3270_emit_copy_state(widget);
    	return;
    }

    // Has clipboard data, inform system.
	GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,terminal->selection.target);

	// Create target list
	//
	// Reference: https://cpp.hotexamples.com/examples/-/-/g_list_insert_sorted/cpp-g_list_insert_sorted-function-examples.html
	//
	GtkTargetList * list = gtk_target_list_new(NULL,0);

	gtk_target_list_add_text_targets(list, CLIPBOARD_TYPE_TEXT);

	if((terminal->selection.options & V3270_SELECTION_PLAIN_TEXT) == 0)
	{
		static const GtkTargetEntry targets[] = {
			{ "text/csv",				 		0, CLIPBOARD_TYPE_CSV				},
			{ "text/html",						0, CLIPBOARD_TYPE_HTML				},
			{ "application/x-v3270-formatted",	0, CLIPBOARD_TYPE_V3270_FORMATTED	},
		};

		gtk_target_list_add_table(list, targets, G_N_ELEMENTS(targets));

	}

	if(terminal->selection.options & V3270_SELECTION_PIXBUFF)
	{
		gtk_target_list_add_image_targets(list,CLIPBOARD_TYPE_PIXBUFF,TRUE);
	}

	int				  n_targets;
	GtkTargetEntry	* targets = gtk_target_table_new_from_list(list, &n_targets);

#ifdef DEBUG
	{
		int ix;
		for(ix = 0; ix < n_targets; ix++) {
			debug("target(%d)=\"%s\"",ix,targets[ix].target);
		}
	}
#endif // DEBUG

	if(gtk_clipboard_set_with_owner(
			clipboard,
			targets,
			n_targets,
			(GtkClipboardGetFunc)	v3270_clipboard_get,
			(GtkClipboardClearFunc) v3270_clipboard_clear,
			G_OBJECT(widget)
		))
	{
		gtk_clipboard_set_can_store(clipboard,targets,1);
	}

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

   	v3270_emit_copy_state(widget);

}


