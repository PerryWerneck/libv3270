/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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

#ifdef _WIN32
	{
		static const int font_size[] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28, 32, 36, 40, 48, 56, 64, 72 };
		size_t ix = 0;

		terminal->font.size = font_size[0];

		// debug("Window: width=%d height=%d",width,height);

		for(ix = 0; ix < G_N_ELEMENTS(font_size); ix++)
		{
			cairo_set_font_size(cr,font_size[ix]);
			cairo_font_extents(cr,&extents);

			/*
			debug("size=%d width=%d height=%d",
						font_size[ix],
						VIEW_WIDTH_FROM_FONT(extents.max_x_advance),
						VIEW_HEIGTH_FROM_FONT( (extents.height+extents.descent) )
					);
			*/

			if( (VIEW_WIDTH_FROM_FONT(extents.max_x_advance)) > width || VIEW_HEIGTH_FROM_FONT((extents.height+extents.descent))  > height) {
				break;
			}

			terminal->font.size	= font_size[ix];
		}

		debug("Selected size=%lf",terminal->font.size);
	}
#else
	{
		double s = 0.1;

		do
		{
			terminal->font.size = s;

			s += 0.1;
			cairo_set_font_size(cr,s);
			cairo_font_extents(cr,&extents);

		} while( (VIEW_HEIGTH_FROM_FONT( (extents.height+extents.descent) ) < height) && (VIEW_WIDTH_FROM_FONT(extents.max_x_advance) < width) );

		debug("Selected size=%lf",terminal->font.size);

	}
#endif // _WIN32	

	// Save scaled font for use on next drawings
	cairo_set_font_size(cr,terminal->font.size);

	if(terminal->font.scaled)
		cairo_scaled_font_destroy(terminal->font.scaled);

	terminal->font.scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(terminal->font.scaled);

 }

