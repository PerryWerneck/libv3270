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

 LIB3270_EXPORT void v3270_copy(GtkWidget *widget, V3270_SELECT_FORMAT format, gboolean cut)
 {

	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	terminal->selection.format = format;

	// Have data? Clear it?
	v3270_clear_clipboard(terminal);

	// Get selection bounds.

	if(lib3270_get_selection_rectangle(terminal->host, &terminal->selection.bounds.row, &terminal->selection.bounds.col, &terminal->selection.bounds.width, &terminal->selection.bounds.height) != 0)
		return;

	debug("Selecion rectangle starts on %u,%u with size of %ux%u",
				terminal->selection.bounds.row, terminal->selection.bounds.col,
				terminal->selection.bounds.width, terminal->selection.bounds.height
		);


	// Copy terminal buffer
	unsigned int r, c;

	terminal->selection.contents = g_new0(struct v3270_character,(terminal->selection.bounds.width * terminal->selection.bounds.height));

	int pos = 0;
	for(r=0;r < terminal->selection.bounds.height; r++)
	{
		// Get starting address.
		int baddr = lib3270_translate_to_address(terminal->host, terminal->selection.bounds.row+r+1, terminal->selection.bounds.col+1);
		if(baddr < 0)
		{
			g_message("Can't convert coordinate %u,%d",terminal->selection.bounds.row+r+1,terminal->selection.bounds.col+1);
			gdk_display_beep(gdk_display_get_default());
			return;
		}

		for(c=0;c < terminal->selection.bounds.width; c++)
		{
			lib3270_get_contents(terminal->host,baddr,baddr,&terminal->selection.contents[pos].chr,&terminal->selection.contents[pos].attr);
			debug("pos=%d baddr=%u char=%c",pos,baddr,terminal->selection.contents[pos].chr);
			pos++;
			baddr++;
		}

	}

	v3270_update_system_clipboard(widget);

 }

