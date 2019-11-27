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
 * Este programa está nomeado como mouse.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <v3270.h>
 #include <terminal.h>
 #include <internals.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

LIB3270_EXPORT void v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action)
{
 	g_return_if_fail(GTK_IS_V3270(widget));
	GTK_V3270(widget)->scroll[((int) direction) & 0x03] = action;
}

gboolean v3270_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
	v3270 * terminal = GTK_V3270(widget);

	lib3270_write_event_trace(terminal->host,"scroll event direction=%d",(int) event->direction);

	if(lib3270_get_program_message(terminal->host) != LIB3270_MESSAGE_NONE || event->direction < 0 || event->direction > G_N_ELEMENTS(terminal->scroll))
	{
		lib3270_write_event_trace(terminal->host,"  dropped (not available)\n");
		return FALSE;
	}

	lib3270_write_event_trace(terminal->host,"\n");

	debug("%d %p", (int) event->direction, terminal->scroll[event->direction]);

	if(terminal->scroll[event->direction])
	{
		debug("%d %s", (int) event->direction, gtk_action_get_name(terminal->scroll[event->direction]));
		gtk_action_activate(terminal->scroll[event->direction]);
		return TRUE;
	}

	return FALSE;
 }
