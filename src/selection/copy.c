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

 static void do_copy(v3270 *terminal, gboolean cut)
 {
 	lib3270_selection * selection = lib3270_get_selection(terminal->host,cut,0);

 	if(selection)
	{
		terminal->selection.blocks = g_list_append(terminal->selection.blocks,selection);
	}

 }

 LIB3270_EXPORT void v3270_clipboard_set(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut)
 {
	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	if(mode != V3270_COPY_APPEND) {

		// It's not append, clear current contents ...
		v3270_clear_selection(terminal);

		// ... and set the new mode.
		terminal->selection.format = mode;

	}

	do_copy(terminal,cut);

	v3270_update_system_clipboard(widget);
 }

 LIB3270_EXPORT void v3270_copy_selection(GtkWidget *widget, V3270_COPY_MODE format, gboolean cut)
 {
	v3270_clipboard_set(widget,format,cut);
 }

 LIB3270_EXPORT void v3270_append_selection(GtkWidget *widget, gboolean cut)
 {
	v3270_clipboard_set(widget,V3270_COPY_APPEND,cut);
 }

