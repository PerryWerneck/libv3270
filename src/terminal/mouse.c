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
 * Este programa está nomeado como mouse.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include <gdk/gdk.h>
 #include <lib3270.h>
 #include <v3270.h>
 #include <terminal.h>
 #include <internals.h>
 #include <lib3270/selection.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/ssl.h>
 #include <v3270/security.h>

 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

/*--[ Implement ]------------------------------------------------------------------------------------*/

gint v3270_get_offset_at_point(v3270 *widget, gint x, gint y) {

	GdkPoint point;
	unsigned int r,c;

	g_return_val_if_fail(widget->font.width > 0,-1);

	if(x > 0 && y > 0) {

		point.x = ((x-widget->font.margin.left)/widget->font.width);
		point.y = ((y-widget->font.margin.top)/widget->font.spacing.value);

		lib3270_get_screen_size(widget->host,&r,&c);

		if(point.x >= 0 && point.y >= 0 && point.x < ((int) c) && point.y < ((int) r))
			return (point.y * c) + point.x;
	}

	return -1;
}

static void single_click(v3270 *widget, int baddr) {

	switch(lib3270_get_selection_flags(widget->host,baddr)) {
	case 0x00:
		// Unselected area, move cursor and remove selection
		v3270_disable_updates(GTK_WIDGET(widget));
		lib3270_set_cursor_address(widget->host,baddr);
		lib3270_unselect(widget->host);
		widget->selecting = 1;
		v3270_enable_updates(GTK_WIDGET(widget));
		break;


	default:
		// Move selected area
		widget->selection.baddr = baddr;
		widget->moving = 1;
	}

}

static void button_1_press(v3270 *terminal, GdkEventType type, int baddr) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wswitch"

	switch(type) {
	case GDK_BUTTON_PRESS: 		// Single click - set mode
		single_click(terminal,baddr);
		break;

	case GDK_2BUTTON_PRESS:		// Double click - Select word
		if(lib3270_select_word_at(terminal->host,baddr))
			lib3270_ring_bell(terminal->host);
		break;

	case GDK_3BUTTON_PRESS:		// Triple clock - Select field
		if(lib3270_select_field_at(terminal->host,baddr))
			lib3270_ring_bell(terminal->host);
		break;

	}

	#pragma GCC diagnostic pop

}

static void button_2_press(v3270 *terminal, GdkEventType type, int baddr) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wswitch"

	switch(type) {
	case GDK_BUTTON_PRESS: 		// Single click - set mode
		if(lib3270_get_selection_flags(terminal->host,baddr)) {

			debug("%s: Center button press over selected area.", __FUNCTION__);
			terminal->copying = 1;

		}
		break;

	case GDK_2BUTTON_PRESS:		// Double click - Select word
		terminal->copying = 0;
		lib3270_unselect(terminal->host);
		break;

	default:
		terminal->copying = 0;

	}

	#pragma GCC diagnostic pop
}

void v3270_emit_popup(v3270 *widget, int baddr, GdkEventButton *event) {
	unsigned char	  chr = 0;
	unsigned short	  attr;
	gboolean		  handled = FALSE;

	lib3270_get_contents(widget->host,baddr,baddr,&chr,&attr);

	v3270_signal_emit(	GTK_WIDGET(widget),
						V3270_SIGNAL_POPUP,
						(attr & LIB3270_ATTR_SELECTED) ? TRUE : FALSE,
						lib3270_is_connected(widget->host) ? TRUE : FALSE,
						event,
						&handled
					);

	if(handled)
		return;

	gdk_display_beep(gtk_widget_get_display(GTK_WIDGET(widget)));
}

static V3270_OIA_FIELD get_field_from_event(v3270 *widget, GdkEventButton *event) {

	if(event->y >= widget->oia.rect->y)
	{
		V3270_OIA_FIELD f;

		for(f=0;f<V3270_OIA_FIELD_COUNT;f++) {
			if(event->x >= widget->oia.rect[f].x && event->x <= (widget->oia.rect[f].x+widget->oia.rect[f].width))
				return f;
		}
	}

	return V3270_OIA_FIELD_INVALID;
}

gboolean v3270_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	int baddr = v3270_get_offset_at_point(GTK_V3270(widget),event->x,event->y);
	v3270 * terminal = GTK_V3270(widget);

	if(baddr >= 0) {

		// Click inside the terminal contents.
		debug("Button %d pressed on terminal addr %d",(int) event->button,baddr);

		terminal->oia.selected = V3270_OIA_FIELD_INVALID;

		switch(event->button) {
		case 1:		// Left button
			button_1_press(terminal,event->type,baddr);
			break;

		case 2:		// Center button
			button_2_press(terminal,event->type,baddr);
			break;

		case 3:		// Right button

			if(event->type == GDK_BUTTON_PRESS)
				v3270_emit_popup(GTK_V3270(widget),baddr,event);
			break;

		}

	} else if(event->type == GDK_BUTTON_PRESS) {

		V3270_OIA_FIELD field = get_field_from_event(GTK_V3270(widget),event);

		if(field != V3270_OIA_FIELD_INVALID) {

			debug("Button %d pressed on OIA %d",(int) event->button, (int) field);

			switch(event->button) {
			case 1:		// Left button
				GTK_V3270(widget)->oia.selected = field;
				break;

			case 3:		// Right button
				{
					gboolean handled = FALSE;

					v3270_signal_emit(
						widget, V3270_SIGNAL_OIA_POPUP,
						(guint) field,
						event,
						&handled
					);

					if(!handled)
						gdk_display_beep(gtk_widget_get_display(widget));

				}
				break;
			}


		}

	}

	return FALSE;
}

gboolean v3270_button_release_event(GtkWidget *widget, GdkEventButton*event) {

	v3270 * terminal = GTK_V3270(widget);

	debug("%s(%d)",__FUNCTION__,(int) event->button);

	switch(event->button) {
	case 1: // Left button
		terminal->selecting	= 0;
		terminal->moving	= 0;
		terminal->resizing	= 0;

		if(terminal->oia.selected != V3270_OIA_FIELD_INVALID && terminal->oia.selected == get_field_from_event(terminal,event))
		{
			gboolean handled = FALSE;
			gboolean connected = lib3270_is_connected(terminal->host) ? TRUE : FALSE;
			V3270_OIA_FIELD field = terminal->oia.selected;

			v3270_signal_emit(
				widget,
				V3270_SIGNAL_FIELD,
				connected,
				field,
				event,
				&handled
			);

			debug("Field click was %s", handled ? "Handled" : "Not handled");

			if(!handled) {

				// The signal was not handled, take default action.
				if(connected && field == V3270_OIA_SSL) {

					// Show the default ssl status dialog.
					debug("%s: Showing the default SSL status dialog",__FUNCTION__);
					v3270_popup_security_dialog(widget);

				} else {
					gdk_display_beep(gtk_widget_get_display(widget));
				}

			}

		}

		terminal->oia.selected = V3270_OIA_FIELD_INVALID;

		break;

	case 2:		// Center button

		if(lib3270_has_selection(terminal->host)) {
			debug("%s: Copy with center button",__FUNCTION__);
			v3270_clipboard_set(widget,V3270_COPY_SMART,FALSE);
		}

		break;

	}

	terminal->copying = 0;

	return FALSE;
}

static void update_mouse_pointer(GtkWidget *widget, int baddr)
{
	v3270	* terminal	= GTK_V3270(widget);
//	int		  id		= 0;

	if(baddr >= 0 && terminal->pointer_id == LIB3270_POINTER_UNLOCKED)
	{
		v3270_set_cursor(widget,lib3270_get_pointer(terminal->host,baddr));
	}
}

void v3270_update_mouse_pointer(GtkWidget *widget)
{
	gint	  x, y;
	gtk_widget_get_pointer(widget,&x,&y);
	update_mouse_pointer(widget,v3270_get_offset_at_point(GTK_V3270(widget),x,y));
}

gboolean v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	v3270		* terminal	= GTK_V3270(widget);
	int			  baddr;

	if(!lib3270_is_connected(terminal->host))
	{
		v3270_set_cursor(widget,LIB3270_POINTER_LOCKED);
		return FALSE;
	}

	baddr = v3270_get_offset_at_point(terminal,event->x,event->y);

	if(baddr >= 0)
	{

		if(terminal->selecting)		// Select region
		{
			lib3270_select_to(terminal->host,baddr);
		}
		if(terminal->moving) 	// Move selected area
		{
			terminal->selection.baddr = lib3270_drag_selection(terminal->host,terminal->pointer,terminal->selection.baddr,baddr);
		}
		else
		{
			terminal->pointer = lib3270_get_selection_flags(terminal->host,baddr);
			update_mouse_pointer(widget,baddr);
		}
	}
	else if(event->y >= terminal->oia.rect->y)
	{
		int id = LIB3270_POINTER_PROTECTED;

		if(event->x >= terminal->oia.rect[V3270_OIA_SSL].x && event->x <= (terminal->oia.rect[V3270_OIA_SSL].x + terminal->oia.rect[V3270_OIA_SSL].width))
		{
			switch(lib3270_get_ssl_state(terminal->host))
			{
			case LIB3270_SSL_UNSECURE:	// No secure connection
				id = LIB3270_POINTER_QUESTION;
				break;

			case LIB3270_SSL_NEGOTIATING:	// Negotiating SSL
			case LIB3270_SSL_VERIFYING:		// Verifying SSL
				id = LIB3270_POINTER_WAITING;
				break;

			case LIB3270_SSL_NEGOTIATED:	/**< Connection secure, no CA or self-signed */
				id = LIB3270_POINTER_QUESTION;
				break;

			case LIB3270_SSL_SECURE:	/**< Connection secure with CA check */
				id = LIB3270_POINTER_QUESTION;
				break;

			default:
				id = LIB3270_POINTER_LOCKED;
			}
		}

		v3270_set_cursor(widget,id);
	}

	return FALSE;
}

