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
 #include <lib3270.h>
 #include <lib3270/log.h>

 #define WIDTH_IN_PIXELS(terminal,x) (x * cols)
 #define HEIGHT_IN_PIXELS(terminal,x) (x * (rows+1))

 #define CONTENTS_WIDTH(terminal) (cols * terminal->font.width)
 #define CONTENTS_HEIGHT(terminal) (((rows+1) * terminal->font.spacing)+OIA_TOP_MARGIN+2)

/*--[ Globals ]--------------------------------------------------------------------------------------*/

#ifdef _WIN32
 const gchar * v3270_default_font = "Lucida Console";
#else
 const gchar * v3270_default_font = "monospace";
#endif // _WIN32

/*--[ Implement ]------------------------------------------------------------------------------------*/

const gchar * v3270_get_default_font_name()
{
	return v3270_default_font;
}

void v3270_update_font_metrics(v3270 *terminal, cairo_t *cr, int width, int height)
{
	// update font metrics
	int rows, cols, hFont, size;

	cairo_font_extents_t extents;

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	terminal->font.weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

	cairo_select_font_face(cr,terminal->font.family, CAIRO_FONT_SLANT_NORMAL,terminal->font.weight);

	if(terminal->font.scaled)
	{
		double w = ((double) width) / ((double)cols);
		double h = ((double) height) / ((double) (rows+2));
		double s = w < h ? w : h;

		cairo_set_font_size(cr,s);
		cairo_font_extents(cr,&extents);

		while( HEIGHT_IN_PIXELS(terminal,(extents.height+extents.descent)) < height && WIDTH_IN_PIXELS(terminal,extents.max_x_advance) < width )
		{
			s += 1.0;
			cairo_set_font_size(cr,s);
			cairo_font_extents(cr,&extents);
		}

		s -= 1.0;

		cairo_set_font_size(cr,s < 1.0 ? 1.0 : s);
		cairo_font_extents(cr,&extents);
	}
	else
	{
		static const int font_size[] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28, 32, 36, 40, 48, 56, 64, 72, 0 };
		int f;

		size = font_size[0];

		for(f=0;font_size[f];f++)
		{
			cairo_set_font_size(cr,font_size[f]);
			cairo_font_extents(cr,&extents);

			if(f == 0)
			{
				terminal->minimum_width  = (cols * extents.max_x_advance);
				terminal->minimum_height = ((rows+1) * (extents.height + extents.descent)) + (OIA_TOP_MARGIN+2);
			}

			if( HEIGHT_IN_PIXELS(terminal,(extents.height+extents.descent)) < height && WIDTH_IN_PIXELS(terminal,extents.max_x_advance) < width )
				size = font_size[f];
		}

		cairo_set_font_size(cr,size);

		#if !GTK_CHECK_VERSION(3,0,0)
			gtk_widget_set_size_request(GTK_WIDGET(terminal),terminal->minimum_width,terminal->minimum_height);
		#endif // !GTK(3,0,0)

	}

	cairo_font_extents(cr,&extents);

	// Save scaled font for use on next drawings
	if(terminal->font.scaled)
		cairo_scaled_font_destroy(terminal->font.scaled);

	terminal->font.scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(terminal->font.scaled);

	cairo_scaled_font_extents(terminal->font.scaled,&extents);

	terminal->font.width    = (int) extents.max_x_advance;
	terminal->font.height   = (int) extents.height;
	terminal->font.ascent   = (int) extents.ascent;
	terminal->font.descent  = (int) extents.descent;

	hFont = terminal->font.height + terminal->font.descent;

	// Create new cursor surface
	if(terminal->cursor.surface)
		cairo_surface_destroy(terminal->cursor.surface);

	terminal->cursor.surface = gdk_window_create_similar_surface(gtk_widget_get_window(GTK_WIDGET(terminal)),CAIRO_CONTENT_COLOR,terminal->font.width,hFont);

	// Center image
	size = CONTENTS_WIDTH(terminal);
	terminal->font.left = (width >> 1) - ((size) >> 1);

	terminal->font.spacing = height / (rows+2);
	if((int) terminal->font.spacing < hFont)
		terminal->font.spacing = hFont;

	size = CONTENTS_HEIGHT(terminal);

	terminal->font.top = (height >> 1) - (size >> 1);

}

LIB3270_EXPORT void v3270_set_font_family(GtkWidget *widget, const gchar *name)
{
	v3270 * terminal;

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);

	if(!name)
	{
		name = v3270_default_font;
	}

	if(g_ascii_strcasecmp(terminal->font.family,name))
	{
		// Font has changed, update it
		g_free(terminal->font.family);

		terminal->font.family = g_strdup(name);
		terminal->font.weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

		g_signal_emit(widget,v3270_widget_signal[SIGNAL_UPDATE_CONFIG], 0, "font-family", name);

		v3270_reload(widget);
		gtk_widget_queue_draw(widget);

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
