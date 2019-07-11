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

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const GtkTargetEntry targets[] =
 {
	{ "COMPOUND_TEXT", 	0, CLIPBOARD_TYPE_TEXT },
	{ "UTF8_STRING", 	0, CLIPBOARD_TYPE_TEXT },
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void clipboard_clear(G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED  GObject *obj)
{
}

static void clipboard_get(G_GNUC_UNUSED  GtkClipboard *clipboard, GtkSelectionData *selection, guint target, GObject *obj)
{
	v3270 * widget = GTK_V3270(obj);

	switch(target)
	{
	case CLIPBOARD_TYPE_TEXT:   /* Get clipboard contents as text */
		if(!widget->selection.text)
        {
			lib3270_ring_bell(widget->host);
        }
		else
        {
            gchar * text = g_convert(widget->selection.text, -1, "UTF-8", lib3270_get_display_charset(widget->host), NULL, NULL, NULL);
			gtk_selection_data_set_text(selection,text,-1);
			g_free(text);
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
void v3270_clear_clipboard(v3270 *terminal)
{
    terminal->selection.text = lib3270_free(terminal->selection.text);
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
    const char *text;

	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

    text = v3270_update_selected_text(widget,cut);

    if(text)
        return g_convert(text, -1, "UTF-8", lib3270_get_display_charset(GTK_V3270(widget)->host), NULL, NULL, NULL);

    return NULL;
}

LIB3270_EXPORT gchar * v3270_get_copy(GtkWidget *widget)
{
    const char *text;
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

    text = GTK_V3270(widget)->selection.text;

    if(!text)
        text = v3270_update_selected_text(widget,FALSE);

    if(text)
        return g_convert(text, -1, "UTF-8", lib3270_get_display_charset(GTK_V3270(widget)->host), NULL, NULL, NULL);

    return NULL;
}

LIB3270_EXPORT void v3270_set_copy(GtkWidget *widget, const gchar *text)
{
	v3270	* terminal;
	gchar   * isotext;

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);
    v3270_clear_clipboard(terminal);

    if(!text)
    {
        /* No string, signal clipboard clear and return */
        g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, FALSE);
        return;
    }

    /* Received text, replace the selection buffer */
    terminal->selection.format = V3270_SELECT_TEXT;
    isotext = g_convert(text, -1, lib3270_get_display_charset(terminal->host), "UTF-8", NULL, NULL, NULL);

    if(!isotext)
    {
        /* No string, signal clipboard clear and return */
        g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, FALSE);
        return;
    }

    terminal->selection.text = lib3270_strdup(isotext);

    g_free(isotext);

    g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, TRUE);
}

void v3270_update_system_clipboard(GtkWidget *widget)
{
	if(GTK_V3270(widget)->selection.text)
	{
        GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,GDK_SELECTION_CLIPBOARD);

		if(gtk_clipboard_set_with_owner(
				clipboard,
				targets,
				G_N_ELEMENTS(targets),
				(GtkClipboardGetFunc)	clipboard_get,
				(GtkClipboardClearFunc) clipboard_clear,
				G_OBJECT(widget)
			))
		{
			gtk_clipboard_set_can_store(clipboard,targets,1);
		}

		g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, TRUE);
	}
}

LIB3270_EXPORT void v3270_copy(GtkWidget *widget, V3270_SELECT_FORMAT mode, gboolean cut)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	GTK_V3270(widget)->selection.format = mode;
	v3270_update_selected_text(widget,cut);
    v3270_update_system_clipboard(widget);
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


