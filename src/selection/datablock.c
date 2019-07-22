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
static GList * getUnprotected(H3270 *hSession, const lib3270_selection *selection)
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
			if((element[col].attribute.visual & LIB3270_ATTR_SELECTED) && !((element[col].attribute.field & LIB3270_FIELD_ATTRIBUTE_PROTECT) || (element[col].attribute.visual & LIB3270_ATTR_MARKER)))
			{
				// Element is selected and not protected, get the length.
				unsigned short start = col;
				unsigned short length = 0;
				while(col < selection->bounds.width)
				{
					if( !(element[col].attribute.visual & (LIB3270_ATTR_SELECTED|LIB3270_ATTR_MARKER)) || (element[col].attribute.field & LIB3270_FIELD_ATTRIBUTE_PROTECT))
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

				struct SelectionFieldHeader * field = (struct SelectionFieldHeader *) g_malloc0(sizeof(struct SelectionFieldHeader) + length);

				field->baddr = lib3270_translate_to_address(hSession,row + selection->bounds.row + 1,start + selection->bounds.col + 1);
				field->length = length;

				// Copy string
				unsigned char *ptr = (unsigned char *) (field+1);
				unsigned short ix;
				for(ix=0;ix < field->length; ix++)
				{
					ptr[ix] = element[start+ix].chr;
				}

				// Add allocated block in the list
				list = g_list_append(list, (gpointer) field);

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
	lib3270_get_screen_size(terminal->host, &header->rows, &header->cols);

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
		debug("Creating block at offset %u", header->length);
		struct SelectionBlockHeader * blockheader = (struct SelectionBlockHeader *) (((unsigned char *) header) + header->length);
		header->length += sizeof(* blockheader);

		blockheader->cursor_address = block->cursor_address;
		blockheader->records = 0;

		// Get values.
		GList * values = getUnprotected(terminal->host, (const lib3270_selection *) element->data);
		GList * value;

		for(value = values; value; value = value->next)
		{
			size_t length = (sizeof(struct SelectionFieldHeader) + ((struct SelectionFieldHeader *) value->data)->length);

			if( (header->length+length) >= szBlock )
			{
				szBlock += ALLOCATION_BLOCK_LENGTH + header->length + length;
				header = (struct SelectionHeader *) g_realloc(header,szBlock+1);
			}

			unsigned char *ptr = ((unsigned char *) header);

			memcpy((ptr+header->length), value->data, length);
			header->length += length;
			blockheader->records++;

		}

		g_list_free_full(values,g_free);

	}

	return (gchar *) g_realloc((gpointer) header, header->length+1);
}

gboolean v3270_set_from_data_block(v3270 * terminal, const struct SelectionHeader *selection)
{
	const unsigned char * raw_data = (const unsigned char *) selection;
	unsigned int		  raw_pos = sizeof(struct SelectionHeader);
	unsigned int		  column;
	unsigned int		  record;

	while(raw_pos < selection->length)
	{
		const struct SelectionBlockHeader * block = (const struct SelectionBlockHeader *) (raw_data + raw_pos);
		debug("Processing block at offset %u with %u elements", raw_pos, block->records);

		raw_pos += sizeof(struct SelectionBlockHeader);
		gboolean found = TRUE;

		for(record = 0; record < block->records; record++)
		{
			const struct SelectionFieldHeader * field = (struct SelectionFieldHeader *) (raw_data + raw_pos);

			debug("Analizing field at %u: addr=%u length=%u",
				raw_pos,
				field->baddr,
				field->length
			);

			raw_pos += (sizeof(struct SelectionFieldHeader) + field->length);
			for(column = 0; column < field->length; column++)
			{
                if(lib3270_is_protected(terminal->host,field->baddr+column))
				{
					debug("Column %d is protected",column);
					found = FALSE;
					break;
				}
			}
		}

		if(found && block->records)
		{
			// The current datablock is valid, paste it!
			raw_data = (const unsigned char *) (block+1);
			raw_pos	 = 0;

			debug("Found valid screen with %u elements", block->records);

			for(record = 0; record < block->records; record++)
			{
				const struct SelectionFieldHeader * field = (struct SelectionFieldHeader *) (raw_data + raw_pos);

				debug("Processing field at %u: addr=%u length=%u",
					raw_pos,
					field->baddr,
					field->length
				);

				raw_pos += (sizeof(struct SelectionFieldHeader) + field->length);

				debug(
					"Pasting record %u baddr=%u length=%u",
						record,
						field->baddr,
						(unsigned int) field->length
				);

				if(lib3270_set_string_at_address(terminal->host, field->baddr,(const unsigned char *) (field+1), field->length) < 0)
				{
					debug("Can't set string baddr=%u length=%u errno=%d %s",
							field->baddr,
							field->length,
							errno,
							strerror(errno)
					);
					return FALSE;
				}
			}

			return TRUE;
		}

	}

	return FALSE;
}
