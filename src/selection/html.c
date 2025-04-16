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

 #include <clipboard.h>
 #include <lib3270/selection.h>
 #include <ctype.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void get_element_colors(v3270 * terminal, unsigned short attr, gchar **fgColor, gchar **bgColor)
{
	GdkRGBA *fg;
	GdkRGBA *bg = terminal->color+((attr & 0x00F0) >> 4);

	if(attr & LIB3270_ATTR_FIELD)
		fg = terminal->color+(attr & 0x0003)+V3270_COLOR_FIELD;
	else
		fg = terminal->color+(attr & 0x000F);

	*fgColor = gdk_rgba_to_string(fg);
	*bgColor = gdk_rgba_to_string(bg);

}

/// @brief Get formatted contents as HTML DIV.
static gchar * get_as_div(v3270 * terminal, const GList *selection, gboolean all, const V3270SelectionOption options) {

	const GList	* element	= selection;
	GString		* string	= g_string_new("");
	gchar 		* bgColor;
	gchar 		* fgColor;

	g_string_append(string,"<div style=\"padding:1em;display:inline-block");

	if(options & V3270_SELECTION_FONT_FAMILY) {
		g_string_append_printf(string,";font-family:%s,monospace",(terminal->selection.font_family ? terminal->selection.font_family : terminal->font.family));
	}

	if(options & V3270_SELECTION_COLORS) {
		bgColor = gdk_rgba_to_string(terminal->color+V3270_COLOR_BACKGROUND);
		g_string_append_printf(string,";background-color:%s",bgColor);
		g_free(bgColor);
	}

	g_string_append(string,"\">");

	while(element) {
		const lib3270_selection * block = ((const lib3270_selection *) element->data);
		unsigned int row, col, src = 0;
		unsigned short flags = block->contents[0].attribute.visual;

		if(options & V3270_SELECTION_COLORS) {

			get_element_colors(terminal,flags,&fgColor,&bgColor);

			g_string_append_printf(
				string,
				"<span style=\"background-color:%s;color:%s\">",
				bgColor,
				fgColor
			);

			g_free(bgColor);
			g_free(fgColor);
		}

#ifdef DEBUG
		g_string_append_c(string,'\n');
#endif // DEBUG

		for(row=0; row < block->bounds.height; row++) {

			for(col=0; col<block->bounds.width; col++) {

				if(flags != block->contents[src].attribute.visual) {

					flags = block->contents[src].attribute.visual;

					if(options & V3270_SELECTION_COLORS) {

						get_element_colors(terminal,flags,&fgColor,&bgColor);

						g_string_append_printf(
							string,
							"</span><span style=\"background-color:%s;color:%s\">",
							bgColor,
							fgColor
						);

						g_free(bgColor);
						g_free(fgColor);
					}

				}

				if( (block->contents[src].attribute.visual & LIB3270_ATTR_SELECTED || all) && !isspace(block->contents[src].chr)) {

					g_string_append_c(string,block->contents[src].chr);

				} else {

					g_string_append(string,((options & V3270_SELECTION_NON_BREAKABLE_SPACE) ? "&nbsp;" : " "));

				}

				src++;

			}
			g_string_append(string,"<br />");
#ifdef DEBUG
			g_string_append_c(string,'\n');
#endif // DEBUG
		}

		if(options & V3270_SELECTION_COLORS) {

			g_string_append(string,"</span>");

		}

		element = g_list_next(element);
	}

#ifdef DEBUG
	g_string_append_c(string,'\n');
#endif // DEBUG

	g_string_append(string,"</div>");

	return g_string_free(string,FALSE);

}

/// @brief Get formatted contents as HTML TABLE.
static gchar * get_as_table(v3270 * terminal, const GList *selection, gboolean all, const V3270SelectionOption G_GNUC_UNUSED(options)) // TODO: Use options to set colors & font.
{
	const GList			* element	= selection;
	GString				* string	= g_string_new("");

	unsigned int		  width		= lib3270_get_width(terminal->host);
	g_autofree gchar	* line		= g_malloc0(width+1);

	GList 				* column;

	g_string_append(string,"<table");

	if(options & V3270_SELECTION_FONT_FAMILY) {
		g_string_append_printf(string," style=\"font-family:%s,monospace\"",(terminal->selection.font_family ? terminal->selection.font_family : terminal->font.family));
	}

	g_string_append(string,"><tbody>");

	// Get contents
	GList * columns = v3270_getColumns_from_selection(terminal, selection, all);

	while(element)
	{
		const lib3270_selection * block = ((const lib3270_selection *) element->data);

		unsigned int row, col, src = 0;

		for(row=0; row < block->bounds.height; row++)
		{

			// Build text line with selected data.
			memset(line,' ',width);
			for(col=0; col<block->bounds.width; col++)
			{
				if(block->contents[src].attribute.visual & LIB3270_ATTR_SELECTED)
				{
					line[block->bounds.col+col] = block->contents[src].chr;
				}

				src++;
			}

			g_string_append(string,"<tr>");

			// Extract columns
			for(column = columns; column; column = column->next)
			{
				struct ColumnDescription * columndescription = (struct ColumnDescription *) column->data;

				g_string_append_printf(string,"<td>");
				g_string_append_len(string,line+columndescription->begin,columndescription->width);
				g_string_append(string,"</td>");

			}

			g_string_append(string,"</tr>");

#ifdef DEBUG
			g_string_append_c(string,'\n');
#endif // DEBUG

		}

		element = g_list_next(element);
	}

	g_list_free_full(columns,g_free);

#ifdef DEBUG
	g_string_append_c(string,'\n');
#endif // DEBUG

	g_string_append(string,"</tbody></table>");

	return g_string_free(string,FALSE);

}

gchar * v3270_get_selection_as_html_div(v3270 * terminal, const GList *selection, const gchar *encoding, gboolean all, const V3270SelectionOption options)
{
	g_autofree char * text = get_as_div(terminal, selection, all, options);
	return g_convert(text, -1, (encoding ? encoding : "UTF-8"), lib3270_get_display_charset(terminal->host), NULL, NULL, NULL);
}

gchar * v3270_get_selection_as_html_table(v3270 * terminal, const GList *selection, const gchar *encoding, gboolean all, const V3270SelectionOption options)
{
	g_autofree char * text = get_as_table(terminal, selection, all, options);
	return g_convert(text, -1, (encoding ? encoding : "UTF-8"), lib3270_get_display_charset(terminal->host), NULL, NULL, NULL);
}

gchar * v3270_get_copy_as_html(v3270 * terminal, const gchar *encoding)
{

	if(terminal->selection.format == V3270_COPY_TABLE)
		return v3270_get_selection_as_html_table(terminal, terminal->selection.blocks, encoding, FALSE, terminal->selection.options);

	return v3270_get_selection_as_html_div(terminal, terminal->selection.blocks, encoding, FALSE, terminal->selection.options);

}
