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

 static void do_copy(v3270 *terminal)
 {
	// Get selection bounds.
	unsigned int row;
	unsigned int col;
	unsigned int width;
	unsigned int height;

	if(lib3270_get_selection_rectangle(terminal->host, &row, &col, &width, &height) != 0)
		return;

	debug("Selecion rectangle starts on %u,%u with size of %ux%u",
				row, col,
				width, height
		);

	// Allocate buffer
	struct selection * selection = g_malloc0(sizeof(struct selection) + (sizeof(struct v3270_character) * (width * height)));

	selection->bounds.row		= row;
	selection->bounds.col		= col;
	selection->bounds.width		= width;
	selection->bounds.height	= height;

	// Copy terminal buffer
	unsigned int r, c;

	int pos = 0;
	for(r=0;r < selection->bounds.height; r++)
	{
		// Get starting address.
		int baddr = lib3270_translate_to_address(terminal->host, selection->bounds.row+r+1, selection->bounds.col+1);
		if(baddr < 0)
		{
			g_message("Can't convert coordinate %u,%d",selection->bounds.row+r+1,selection->bounds.col+1);
			gdk_display_beep(gdk_display_get_default());
			return;
		}

		for(c=0;c < selection->bounds.width; c++)
		{
			lib3270_get_contents(terminal->host,baddr,baddr,&selection->contents[pos].chr,&selection->contents[pos].attr);
			debug("pos=%d baddr=%u char=%c",pos,baddr,selection->contents[pos].chr);
			pos++;
			baddr++;
		}

	}

	terminal->selection.blocks = g_list_append(terminal->selection.blocks,selection);

 }

 LIB3270_EXPORT void v3270_copy_selection(GtkWidget *widget, V3270_SELECT_FORMAT format, gboolean cut)
 {
	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	// Have data? Clear it?
	v3270_clear_clipboard(terminal);

	terminal->selection.format = format;
	do_copy(terminal);

	v3270_update_system_clipboard(widget);

	if(cut)
	{
		lib3270_erase_selected(terminal->host);
	}
 }

 LIB3270_EXPORT void v3270_append_selection(GtkWidget *widget, gboolean cut)
 {
	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	do_copy(terminal);

	v3270_update_system_clipboard(widget);

	if(cut)
	{
		lib3270_erase_selected(terminal->host);
	}
 }

