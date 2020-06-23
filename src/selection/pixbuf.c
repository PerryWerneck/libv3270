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

 #include <clipboard.h>
 #include <lib3270/selection.h>
 #include <v3270/dialogs.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

GdkPixbuf * v3270_get_selection_as_pixbuf(v3270 * terminal, const GList *selections, gboolean all)
{
	const GList *selection;

	// Get image size
	size_t rows = 0, cols = 0;

	for(selection = selections; selection; selection = g_list_next(selection))
	{
		lib3270_selection * block = ((lib3270_selection *) selection->data);
		unsigned int row, col, src = 0;

		for(row=0; row < block->bounds.height; row++)
		{
			size_t hasSelection = FALSE;

			for(col=0; col<block->bounds.width; col++)
			{
				if( (block->contents[src].attribute.visual & LIB3270_ATTR_SELECTED) || all )
				{
					hasSelection = TRUE;
					if(col > cols)
						cols = col;
				}
			}

			if(hasSelection)
			{
				rows++;
			}

		}

	}

	if(!rows)
		return NULL;

	debug("Selection rows=%u cols=%u",(unsigned int) rows, (unsigned int) cols);

	gint width	= (cols+1) * terminal->font.width;
	gint height	= (rows+1) * terminal->font.spacing.value;

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

	cairo_t *cr = cairo_create(surface);
	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_BACKGROUND);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
	cairo_stroke(cr);

	// Draw contents
	GdkRectangle rect;
	memset(&rect,0,sizeof(rect));
	rect.width	= terminal->font.width;
	rect.height	= terminal->font.spacing.value;

	cairo_set_scaled_font(cr,terminal->font.scaled);

	for(selection = selections; selection; selection = g_list_next(selection))
	{
		lib3270_selection * block = ((lib3270_selection *) selection->data);
		unsigned int row, col, src = 0;

		for(row=0; row < block->bounds.height; row++)
		{
			size_t hasSelection = FALSE;
			rect.x = 0;

			for(col=0; col<block->bounds.width; col++)
			{
				if( (block->contents[src].attribute.visual & LIB3270_ATTR_SELECTED) || all )
				{
					hasSelection = TRUE;

					v3270_draw_element(
						cr,
						block->contents[src].chr,
						block->contents[src].attribute.visual & ~LIB3270_ATTR_SELECTED,
						terminal->host,
						&terminal->font,
						&rect,
						terminal->color
					);

				}
				src++;
				rect.x += rect.width;
			}

			if(hasSelection)
			{
				rect.y += terminal->font.spacing.value;
			}

		}
	}

	cairo_destroy (cr);

	GdkPixbuf * pixbuf = gdk_pixbuf_get_from_surface(surface,0,0,width,height);

	cairo_surface_destroy (surface);

	return pixbuf;

}
