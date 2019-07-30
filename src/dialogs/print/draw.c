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
 #include <string.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static gint get_row(const V3270PrintOperation * operation, gint row, const lib3270_selection **block)
 {
	const GList					* element = NULL;
	const lib3270_selection		* selection = NULL;

	debug("Searching for row %u", row);

	for(element = operation->contents.selection; element; element = element->next)
	{
		selection = (const lib3270_selection *) element->data;

		if(((unsigned int) row) < selection->bounds.height)
		{
			// Found block.
			debug("Found row %u",row);
			*block = selection;
			return row;
		}

		debug("Skipping block with %u rows",selection->bounds.height);
		row -= (int) selection->bounds.height;
	}

	return -1;
 }

 void V3270PrintOperation_draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint page)
 {
	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(prt);
 	cairo_t				* cr = gtk_print_context_get_cairo_context(context);

 	// Convert page number in rows.
    gint row = page * operation->lpp;
    if(((unsigned int) row) > operation->contents.height)
		return;

	debug("%s: page=%d skip_rows=%d",__FUNCTION__,page,row);

	// Get block
	const lib3270_selection	* selection = NULL;
	row = get_row(operation,row,&selection);

	// Setup a rectangle with the size of 1 character.
	GdkRectangle rect;
	memset(&rect,0,sizeof(rect));
	rect.y 		= 2;
	rect.height	= operation->font.info.height + operation->font.info.descent;
	rect.width	= operation->font.info.width;

	// Clear drawing area.
	gdk_cairo_set_source_rgba(cr,operation->settings->colors + V3270_COLOR_BACKGROUND);
	cairo_rectangle(
			cr,
			operation->font.info.left-1,0,
			(rect.width * operation->contents.width) + 4,
			(rect.height * operation->contents.height) + 4
	);

	cairo_fill(cr);
	cairo_stroke(cr);

	// Draw LPP lines
	size_t drawing;

	for(drawing = 0; drawing < operation->lpp; drawing++)
	{
		if(((unsigned int) row) >= selection->bounds.height)
		{
			debug("Searching for next block (first line=%u)",(unsigned int) (page * operation->lpp) + drawing);
			row = get_row(operation,(page * operation->lpp) + drawing, &selection);
			if(row < 0)
				break;

		}

		// Draw columns
		size_t pos = (row * selection->bounds.width);
		size_t col;
		rect.x = operation->font.info.left;

		debug("Drawing: %u row=%u selection=%p pos=%u", (unsigned int) drawing, row, selection, (unsigned int) pos);

		for(col = 0; col < selection->bounds.width;col++)
		{
			if(selection->contents[pos].chr)
			{
				// Draw character.
				unsigned short attr = selection->contents[pos].attribute.visual;

				if(!operation->show_selection)
					attr &= ~LIB3270_ATTR_SELECTED;

				v3270_draw_element(
					cr,
					selection->contents[pos].chr,
					attr,
					operation->session,
					&operation->font.info,
					&rect,
					operation->settings->colors
				);

			}
            pos++;

			// Advance to the next char
			rect.x += (rect.width-1);

		}

		// Advance to the next row
		rect.y += (rect.height-1);
		row++;

	}

 	/*


	// Create a rectangle with the size of 1 character.

	// Draw "operation->lpp" lines starting from "from"

	// Clear contents.

	// draw "lpp" lines starting from "from"
    size_t r;

	cairo_set_scaled_font(cr,operation->font.info.scaled);

    for(r = 0; r < operation->lpp; r++)
	{
		rect.x = operation->font.info.left;

		size_t row = r+from;
		if(row > operation->contents.height || !operation->contents.text[row])
			break;

		size_t col;
		column * columns = operation->contents.text[row];
		for(col = 0; col < operation->contents.width; col++)
		{
			if(columns[col].c)
			{
				// Draw character
				if(!operation->show_selection)
					columns[col].attr &= ~LIB3270_ATTR_SELECTED;

				v3270_draw_element(
					cr,
					columns[col].c,
					columns[col].attr,
					operation->session,
					&operation->font.info,
					&rect,
					operation->colors
				);

			}

			// Advance to the next char
			rect.x += (rect.width-1);

		}

		// Advance to the next row
		rect.y += (rect.height-1);

	}
	*/

 }
