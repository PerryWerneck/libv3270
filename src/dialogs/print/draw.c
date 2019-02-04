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

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void V3270PrintOperation_draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint page)
 {
 	cairo_t				* cr = gtk_print_context_get_cairo_context(context);
	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(prt);

    size_t from = page * operation->lpp;

    if(from > operation->contents.height)
		return;

	// Create a rectangle with the size of 1 character.
	GdkRectangle rect;
	memset(&rect,0,sizeof(rect));
	rect.y 		= 2;
	rect.height	= operation->font.info.height + operation->font.info.descent;
	rect.width	= operation->font.info.width;

	// Draw "operation->lpp" lines starting from "from"

	// Clear contents.
	gdk_cairo_set_source_rgba(cr,operation->colors + V3270_COLOR_BACKGROUND);
	cairo_rectangle(
			cr,
			operation->font.info.left-1,0,
			(rect.width * operation->contents.width) + 4,
			(rect.height * operation->contents.height) + 4
	);

	cairo_fill(cr);
	cairo_stroke(cr);

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

 }
