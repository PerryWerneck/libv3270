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

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void V3270PrintOperation_begin_print(GtkPrintOperation *prt, GtkPrintContext *context)
 {
 	cairo_t				* cr = gtk_print_context_get_cairo_context(context);
	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(prt);

 	trace("%s",__FUNCTION__);

 	// Setup FONT
	PangoFontDescription * fontDescription = pango_font_description_from_string(operation->font.name);

	cairo_select_font_face(
		cr,
		pango_font_description_get_family(fontDescription),
		CAIRO_FONT_SLANT_NORMAL,
		pango_font_description_get_weight(fontDescription) == PANGO_WEIGHT_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
	);

	// Set font size based on text and context.
   	cairo_font_extents_t    extents;
	double                  width    = gtk_print_context_get_width(context);
	double                  cols     = (double) operation->contents.width;
	double                  current  = width / cols;
	double                  valid    = current;

	do
	{
		valid   = current;
		current = valid +1.0;
		cairo_set_font_size(cr,current);
		cairo_font_extents(cr,&extents);
	}
	while(  (cols * extents.max_x_advance) < width );

	trace("Font size: %d",(int) valid);
	cairo_set_font_size(cr,valid);

	// Store font data.
	operation->font.info.scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(operation->font.info.scaled);
	cairo_scaled_font_extents(operation->font.info.scaled,&extents);

	operation->font.info.height		= extents.height;
	operation->font.info.descent	= extents.descent;
	operation->font.info.width		= extents.max_x_advance;

	operation->font.info.width++;

	// Center text on page
	// operation->font.info.left = 2;

	operation->font.info.left = (gtk_print_context_get_width(context)- (operation->font.info.width * operation->contents.width))/2;
	if(operation->font.info.left < 2)
		operation->font.info.left = 2;

	// Setup page size
	operation->lpp	= (gtk_print_context_get_height(context) / (extents.height + extents.descent));
	operation->pages = (operation->contents.height / operation->lpp)+1;

	trace("%d lines per page, %d pages to print",operation->lpp,operation->pages);

	gtk_print_operation_set_n_pages(prt,operation->pages);

 }

