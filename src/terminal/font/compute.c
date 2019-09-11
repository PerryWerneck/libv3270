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
 * @brief Compute font size.
 *
 * @param terminal	v3270 terminal widget.
 * @param cr		Cairo context.
 * @param width		Surface width in pixels.
 * @param height	Surface height in pixels.
 *
 */
 void v3270_compute_font_size(v3270 *terminal, cairo_t *cr, unsigned int width, unsigned int height)
 {
	unsigned int rows, cols;
	cairo_font_extents_t extents;

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	/*
	debug(
		"Screen_size: %ux%u Scalled=%s view_rows=%d",
			rows,
			cols,
			terminal->font.scaled ? "Yes" : "No",
			(rows+OIA_TOP_MARGIN+3)
	);
	*/

	terminal->font.weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

	if(terminal->font.face)
		cairo_font_face_destroy(terminal->font.face);

	terminal->font.face = cairo_toy_font_face_create(terminal->font.family, CAIRO_FONT_SLANT_NORMAL, terminal->font.weight);
	cairo_set_font_face(cr,terminal->font.face);

	{
		double s = terminal->zoom.step;
		double selected = 0;

		do
		{
			selected = s;

			s += terminal->zoom.step;
			cairo_set_font_size(cr,s);
			cairo_font_extents(cr,&extents);

		} while( (VIEW_HEIGTH_FROM_FONT( (extents.height+extents.descent) ) < height) && (VIEW_WIDTH_FROM_FONT(extents.max_x_advance) < width) );

		debug("Selected size=%lf",selected);

		cairo_set_font_size(cr,selected);
		cairo_font_extents(cr,&extents);

	}

	// Save scaled font for use on next drawings
	if(terminal->font.scaled)
		cairo_scaled_font_destroy(terminal->font.scaled);

	terminal->font.scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(terminal->font.scaled);

 }

