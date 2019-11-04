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

/*--[ Implement ]------------------------------------------------------------------------------------*/

/**
 * @brief Update font metrics based on view sizes.
 *
 * @param terminal	v3270 terminal widget.
 * @param cr		Cairo context.
 * @param width		View width in pixels.
 * @param height	View height in pixels.
 *
 */
void v3270_update_font_metrics(v3270 *terminal, unsigned int width, unsigned int height)
{
	unsigned int rows, cols, size;
	cairo_font_extents_t extents;

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	cairo_scaled_font_extents(terminal->font.scaled,&extents);

	// Update margins.

	terminal->font.width    		= (int) extents.max_x_advance;
	terminal->font.height   		= (int) extents.height;
	terminal->font.ascent   		= (int) extents.ascent;
	terminal->font.descent  		= (int) extents.descent;
	terminal->font.spacing.value	= terminal->font.height + terminal->font.descent;

	if(terminal->font.spacing.dynamic)
	{
		// Compatibility adjustments for line spacing.

		// Some users complained about the new fixed line spacing; for them
		// the old style is bettter!

		guint spacing = height / (rows+2);
		if(spacing > terminal->font.spacing.value)
			terminal->font.spacing.value = spacing;
	}

	// Create new cursor surface
	if(terminal->cursor.surface)
		cairo_surface_destroy(terminal->cursor.surface);

	terminal->cursor.surface = gdk_window_create_similar_surface(gtk_widget_get_window(GTK_WIDGET(terminal)),CAIRO_CONTENT_COLOR,terminal->font.width,terminal->font.spacing.value);

	// Center image

	size = VIEW_WIDTH_FROM_FONT(terminal->font.width);
	terminal->font.margin.left = (width/2) - (size/2);

	debug("%d",(width - size));

	size = VIEW_HEIGTH_FROM_FONT(terminal->font.spacing.value);
	terminal->font.margin.top = (height/2) - (size/2);

	debug("%d",(height - size));

	debug("screen_height=%u content_height=%d top=%d left=%d size=%lf",height, size, terminal->font.margin.top, terminal->font.margin.left, terminal->font.size);

}

