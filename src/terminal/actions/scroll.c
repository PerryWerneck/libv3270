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
 #include <v3270/actions.h>
 #include <terminal.h>
 #include <internals.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <gdk/gdkkeysyms-compat.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

// Callback for compatibility with the old application.
static void activate_action(GtkWidget G_GNUC_UNUSED(*terminal), GtkAction *action)
{
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	gtk_action_activate(action);
	#pragma GCC diagnostic pop
}

LIB3270_EXPORT void v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action)
{
	v3270_set_scroll_handler(widget, direction, G_CALLBACK(activate_action), action);
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

	// Do I have scroll method??
	if(terminal->scroll[event->direction].activate)
	{
		// Yes, fire it.
		terminal->scroll[event->direction].activate(widget,terminal->scroll[event->direction].arg);
		return TRUE;
	}

	// Check for fallbacks
	size_t ix;

 	static const struct
 	{
		GdkScrollDirection	direction;
		guint				keyval;
		GdkModifierType		state;
 	} fallbacks[] = {

		{ GDK_SCROLL_UP,		GDK_Page_Up,		0					},
		{ GDK_SCROLL_DOWN,		GDK_Page_Down,		0					},

		{ GDK_SCROLL_UP,		GDK_ISO_Left_Tab,	GDK_SHIFT_MASK		},
		{ GDK_SCROLL_DOWN,		GDK_Tab,			0					},

		{ GDK_SCROLL_UP,		GDK_Up,				0					},
		{ GDK_SCROLL_DOWN,		GDK_Down,			0					},

 	};

	for(ix = 0; ix < G_N_ELEMENTS(fallbacks); ix++) {

		if(fallbacks[ix].direction == event->direction) {

			const V3270Accelerator * accel = v3270_get_accelerator(widget, fallbacks[ix].keyval, fallbacks[ix].state);
			if(accel)
			{
				debug("Activating accelerator \"%s\"\n",v3270_accelerator_get_name(accel));
//				lib3270_write_event_trace(terminal->hSession,"Activating fallback mouse action \"%s\"\n",v3270_accelerator_get_name(accel));
				v3270_accelerator_activate(accel,widget);
				return TRUE;
			}

		}

	}

	/*

	// Has selection?
	if(lib3270_has_selection(terminal->host))
	{
        switch(event->direction)
        {
		case GDK_SCROLL_UP:
			lib3270_move_selection(terminal->host,LIB3270_DIR_UP);
			return TRUE;

		case GDK_SCROLL_DOWN:
			lib3270_move_selection(terminal->host,LIB3270_DIR_DOWN);
			return TRUE;

		case GDK_SCROLL_LEFT:
			lib3270_move_selection(terminal->host,LIB3270_DIR_LEFT);
			return TRUE;

		case GDK_SCROLL_RIGHT:
			lib3270_move_selection(terminal->host,LIB3270_DIR_RIGHT);
			return TRUE;

		case GDK_SCROLL_SMOOTH:
			return FALSE;
        }

	}

	*/

	return FALSE;
 }

 void v3270_set_scroll_handler(GtkWidget *widget, GdkScrollDirection direction, GCallback callback, gpointer data)
 {
 	size_t ix = (size_t) direction;

 	g_return_if_fail(GTK_IS_V3270(widget) && ix < 4);

	v3270 * terminal = GTK_V3270(widget);

	terminal->scroll[ix].activate = (void (*)(GtkWidget *, gpointer)) callback;
	terminal->scroll[ix].arg = data;

 }

