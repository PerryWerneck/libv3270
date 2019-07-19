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

 #define ALLOCATION_BLOCK_LENGTH 4096

/*--[ Implement ]------------------------------------------------------------------------------------*/

/// @brief Get a list of all selected and unprotected contents.
static GList * getUnprotected(const lib3270_selection *selection)
{
	GList * list = NULL;
	unsigned int row;
	const lib3270_selection_element * element = selection->contents;

	for(row=0; row < selection->bounds.height; row++)
	{
		unsigned int col = 0;

		// Find selected and unprotected entries.
		while(col < selection->bounds.width)
		{
			if((element[col].attribute.visual & LIB3270_ATTR_SELECTED) && !(element[col].attribute.field & LIB3270_FIELD_ATTRIBUTE_PROTECT))
			{
				// Element is selected and not protected, get the length.
				unsigned short start = col;
				unsigned short length = 0;
				while(col < selection->bounds.width)
				{
					if( !(element[col].attribute.visual & LIB3270_ATTR_SELECTED) || (element[col].attribute.field & LIB3270_FIELD_ATTRIBUTE_PROTECT))
						break;

					col++;
					length++;
				}

				debug(
					"Row:%u Col: %u Length: %u",
					row + selection->bounds.row,
					start + selection->bounds.col,
					(unsigned int) length
				);

			}
			else
			{
				col++;
			}

		}

		element += selection->bounds.width;

	}

	return list;
}

gchar * v3270_get_copy_as_data_block(v3270 * terminal)
{

	GList					* element;
	size_t					  szBlock = ALLOCATION_BLOCK_LENGTH + sizeof(struct SelectionHeader);
	struct SelectionHeader	* header = (struct SelectionHeader *) g_malloc0(szBlock+1);

	// Initialize header.
	header->build 	= BUILD_DATE;
	header->length	= sizeof(struct SelectionHeader);

	// Insert elements.
	for(element = terminal->selection.blocks; element; element = element->next)
	{
		lib3270_selection * block = ((lib3270_selection *) element->data);
		size_t length = (block->bounds.height * block->bounds.width);

		if( (header->length+length) >= szBlock )
		{
			szBlock += ALLOCATION_BLOCK_LENGTH + header->length + length;
			header = (struct SelectionHeader *) g_realloc(header,szBlock+1);
		}

		// Setup block header
		struct SelectionBlockHeader * blockheader = (struct SelectionBlockHeader *) (((unsigned char *) header) + header->length);
		header->length += sizeof(* blockheader);

		blockheader->cursor_address = block->cursor_address;
		blockheader->records = 0;

		// Get values.
		GList * values = getUnprotected((const lib3270_selection *) element->data);


		g_list_free_full(values,g_free);

	}

	return (gchar *) g_realloc((gpointer) header, header->length+1);
}
