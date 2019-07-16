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
 #include <ctype.h>
 #include <lib3270/selection.h>

 struct ColumnDescription {
	unsigned int begin;
	unsigned int width;
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

/// @brief Check if column has data.
static gboolean hasDataOnColumn(v3270 * terminal, unsigned int col)
{
	GList * element	= terminal->selection.blocks;

	while(element)
	{
		lib3270_selection * block = ((lib3270_selection *) element->data);

		if( (col >= block->bounds.col) && ( col < (block->bounds.col + block->bounds.width)) )
		{
			unsigned int pos = col-block->bounds.col;
			unsigned int row;

			for(row = 0; row < block->bounds.height; row++)
			{
				if( (block->contents[pos].flags & LIB3270_ATTR_SELECTED) && !isspace(block->contents[pos].chr))
				{
					return TRUE;
				}
				pos += block->bounds.width;
			}

		}

		element = g_list_next(element);
	}

	return FALSE;
}

/// @brief Get column list.
GList * getColumns(v3270 * terminal)
{
	unsigned int col = 0;
	GList *rc = NULL;

	while(col < lib3270_get_width(terminal->host)) {

		// debug("col(%u): %s", col, hasDataOnColumn(terminal,col) ? "yes" : "no");

		// Get first column.
		while(!hasDataOnColumn(terminal,col)) {
			if(col >= lib3270_get_width(terminal->host))
				return rc;
			col++;
		}

		// Alocate block, add it to list.
		struct ColumnDescription * columndescription = g_new0(struct ColumnDescription,1);
		columndescription->begin = col;
		rc = g_list_append(rc,columndescription);

		// Get width.
		while(hasDataOnColumn(terminal,col++)) {
			columndescription->width++;
			if(col >= lib3270_get_width(terminal->host))
				return rc;
		}
	}

	return rc;

}

/// @brief Get formatted contents as single text.
gchar * v3270_get_copy_as_table(v3270 * terminal, const gchar *delimiter)
{
	GString	* string = g_string_new("");

	GList * columns = getColumns(terminal);

	debug("columns=%p",columns);

#ifdef DEBUG
	{
		GList * column = columns;
		while(column)
		{
			struct ColumnDescription * columndescription = (struct ColumnDescription *) column->data;

            debug("Begin: %u Width: %u",columndescription->begin, columndescription->width);

			column = column->next;
		}
	}
#endif // DEBUG

	GList				* element	= terminal->selection.blocks;
	unsigned int		  width		= lib3270_get_width(terminal->host);
	g_autofree gchar	* line		= g_malloc0(width+1);
	GList 				* column;

	while(element)
	{
		lib3270_selection * block = ((lib3270_selection *) element->data);

		unsigned int row, col, src = 0;

		for(row=0; row < block->bounds.height; row++)
		{

			// Build text line with selected data.
			memset(line,' ',width);
			for(col=0; col<block->bounds.width; col++)
			{
				if(block->contents[src].flags & LIB3270_ATTR_SELECTED)
				{
					line[block->bounds.col+col] = block->contents[src].chr;
				}

				src++;
			}

			debug("[%s]",line);

			// Extract columns
			for(column = columns; column; column = column->next)
			{
				struct ColumnDescription * columndescription = (struct ColumnDescription *) column->data;
				g_string_append_len(string,line+columndescription->begin,columndescription->width);
				if(column->next)
					g_string_append(string,delimiter);
			}
			g_string_append(string,"\n");

		}

		element = g_list_next(element);
	}

	g_list_free_full(columns,g_free);

	g_autofree char * text = g_string_free(string,FALSE);
	return g_convert(text, -1, "UTF-8", lib3270_get_display_charset(terminal->host), NULL, NULL, NULL);
}
