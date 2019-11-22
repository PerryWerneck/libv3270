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

 #include <config.h>
 #include "private.h"
 #include <gdk/gdk.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/


/*--[ Implement ]------------------------------------------------------------------------------------*/

const gchar * v3270_get_default_font_name()
{
#ifdef _WIN32
	return "Lucida Console";
#else
	return "monospace";
#endif // _WIN32
}

static guint validate_font_family(const gchar *family_name)
{
	int rc = 2;

	gint n_families, i;
	PangoFontFamily **families;

	PangoContext * context = gdk_pango_context_get_for_screen(gdk_screen_get_default());

	pango_context_list_families(context,&families, &n_families);

	for(i=0; i < n_families; i++)
	{
		if(!g_ascii_strcasecmp(pango_font_family_get_name(families[i]),family_name))
		{
			rc = pango_font_family_is_monospace(families[i]) ? 0 : 1;
			break;
		}


	}
	g_object_unref(G_OBJECT(context));
	g_free(families);

	return rc;
}

LIB3270_EXPORT void v3270_set_font_family(GtkWidget *widget, const gchar *name)
{
	v3270 * terminal;

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);

	if(!name)
		name = v3270_get_default_font_name();

	if(g_ascii_strcasecmp(terminal->font.family,name))
	{

		switch(validate_font_family(name))
		{
		case 0:
			g_message("Change font to \"%s\"", name);
			break;

		case 1:
			g_warning("Font \"%s\" is not monospace", name);
			break;

		default:
			g_warning("Invalid or unexpected font family name: \"%s\"", name);

		}

		// Font has changed, update it
		g_free(terminal->font.family);

		terminal->font.family = g_strdup(name);
		terminal->font.weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

		g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_UPDATE_CONFIG], 0, "font-family", name);

		debug("%s: %p",__FUNCTION__,GTK_V3270_GET_CLASS(widget)->properties.font_family);
		g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.font_family);

		if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
		{
			v3270_reconfigure(terminal);
			gtk_widget_queue_draw(widget);
		}

	}

}

LIB3270_EXPORT const gchar * v3270_get_font_family(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return GTK_V3270(widget)->font.family;
}

LIB3270_EXPORT void v3270_set_scaled_fonts(GtkWidget *widget, gboolean on)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GTK_V3270(widget)->scaled_fonts = on ? 1 : 0;

}
