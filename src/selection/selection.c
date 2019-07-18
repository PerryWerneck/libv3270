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
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void clipboard_clear(G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED  GObject *obj)
{
	v3270 * terminal = GTK_V3270(obj);

	if(!lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_KEEP_SELECTED))
	{
		v3270_unselect(GTK_WIDGET(obj));
		v3270_clear_selection(terminal);
	}

}

static void clipboard_get(G_GNUC_UNUSED  GtkClipboard *clipboard, GtkSelectionData *selection, guint target, GObject *obj)
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
			gchar *text;

			if(terminal->selection.format == V3270_SELECT_TABLE)
			{
				text = v3270_get_copy_as_table(terminal,"\t");
			}
			else
			{
				text = v3270_get_copy_as_text(terminal);
			}
			gtk_selection_data_set_text(selection,text,-1);
			g_free(text);
		}
		break;

	case CLIPBOARD_TYPE_CSV:
		{
			g_autofree gchar *text = v3270_get_copy_as_table(terminal,";");
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

	default:
		g_warning("Unexpected clipboard type %d\n",target);
	}
}

/**
 * Clear clipboard contents.
 *
 * @param terminal	Pointer to the terminal Widget.
 *
 */
void v3270_clear_selection(v3270 *terminal)
{
	if(terminal->selection.blocks)
	{
		g_list_free_full(terminal->selection.blocks,(GDestroyNotify) lib3270_free);
		terminal->selection.blocks = NULL;
	}
}

/**
 * Get lib3270 selection as a g_malloc buffer.
 *
 * @param widget	Widget containing the desired section.
 *
 * @return NULL if error, otherwise the selected buffer contents (release with g_free).
 *
 */
LIB3270_EXPORT gchar * v3270_get_selected(GtkWidget *widget, gboolean cut)
{
	lib3270_autoptr(char) text = NULL;

	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	if(cut)
		text = lib3270_cut_selected(GTK_V3270(widget)->host);
	else
		text = lib3270_get_selected(GTK_V3270(widget)->host);

    if(text)
        return g_convert(text, -1, "UTF-8", lib3270_get_display_charset(GTK_V3270(widget)->host), NULL, NULL, NULL);

    return NULL;
}

void v3270_update_system_clipboard(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

    if(!terminal->selection.blocks)
    {
    	// No clipboard data, return.
		g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, FALSE);
    	return;
    }

    // Has clipboard data, inform system.
	GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,terminal->selection.target);

	// Create target list
	//
	// Reference: https://cpp.hotexamples.com/examples/-/-/g_list_insert_sorted/cpp-g_list_insert_sorted-function-examples.html
	//
	static const GtkTargetEntry internal_targets[] = {
		{ "text/csv", 0, CLIPBOARD_TYPE_CSV }
	};

	GtkTargetList 	* list = gtk_target_list_new(internal_targets, G_N_ELEMENTS(internal_targets));
	GtkTargetEntry	* targets;
	int				  n_targets;

	gtk_target_list_add_text_targets(list, CLIPBOARD_TYPE_TEXT);

	targets = gtk_target_table_new_from_list(list, &n_targets);

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
			(GtkClipboardGetFunc)	clipboard_get,
			(GtkClipboardClearFunc) clipboard_clear,
			G_OBJECT(widget)
		))
	{
		gtk_clipboard_set_can_store(clipboard,targets,1);
	}

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

	g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, TRUE);

}

LIB3270_EXPORT void v3270_unselect(GtkWidget *widget)
{
	v3270_disable_updates(widget);
	lib3270_unselect(v3270_get_session(widget));
	v3270_enable_updates(widget);
}

gboolean v3270_get_selection_bounds(GtkWidget *widget, gint *start, gint *end)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return lib3270_get_selection_bounds(GTK_V3270(widget)->host,start,end) == 0 ? FALSE : TRUE;
}

LIB3270_EXPORT  void v3270_select_region(GtkWidget *widget, gint start, gint end)
{
 	g_return_if_fail(GTK_IS_V3270(widget));
 	lib3270_select_region(GTK_V3270(widget)->host,start,end);
}

LIB3270_EXPORT void v3270_select_all(GtkWidget *widget)
{
 	g_return_if_fail(GTK_IS_V3270(widget));
	v3270_disable_updates(widget);
	lib3270_select_all(v3270_get_session(widget));
	v3270_enable_updates(widget);
}


