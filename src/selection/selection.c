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
		#pragma GCC diagnostic push
#ifdef _WIN32
		#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif // _WIN32

		g_list_free_full(terminal->selection.blocks,(GDestroyNotify) lib3270_free);

		#pragma GCC diagnostic pop

		terminal->selection.blocks = NULL;
	}
}

const GList * v3270_get_selection_blocks(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return GTK_V3270(widget)->selection.blocks;
}

gboolean v3270_has_copy(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return GTK_V3270(widget)->selection.blocks != NULL;
}

void v3270_emit_copy_state(GtkWidget *widget)
{
	gboolean has_copy = (GTK_V3270(widget)->selection.blocks != NULL);

	debug("%s(%s)",__FUNCTION__,has_copy ? "TRUE" : "FALSE");

	g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, has_copy);
	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.has_copy);

	lib3270_set_has_copy(GTK_V3270(widget)->host, has_copy ? 1 : 0);

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


