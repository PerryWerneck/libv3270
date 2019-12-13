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
 * Este programa está nomeado como keyboard.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

 #include <lib3270.h>
 #include <lib3270/actions.h>
 #include <v3270/actions.h>
 #include <lib3270/log.h>
 #include <gtk/gtk.h>
 #include <string.h>
 #include <gdk/gdk.h>

 #include <v3270.h>
 #include <internals.h>
 #include <terminal.h>

#if GTK_CHECK_VERSION(3,0,0)
	#include <gdk/gdkkeysyms-compat.h>
#else
	#include <gdk/gdkkeysyms.h>
#endif

#ifndef GDK_ALT_MASK
	#define GDK_ALT_MASK GDK_MOD1_MASK
#endif

#ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
#endif

/*--[ Implement ]------------------------------------------------------------------------------------*/

 #define keyval_is_alt() (event->keyval == GDK_Alt_L || event->keyval == GDK_Meta_L || event->keyval == GDK_ISO_Level3_Shift)

 static void update_keyboard_state(v3270 *terminal, GdkEventKey *event, gboolean status)
 {
	if(event->keyval == GDK_Shift_R || event->keyval == GDK_Shift_L)
	{
		if(status)
			terminal->keyflags |= KEY_FLAG_SHIFT;
		else
			terminal->keyflags &= ~KEY_FLAG_SHIFT;
		v3270_draw_shift_status(terminal);
	}

#ifdef KEY_FLAG_ALT
	if(keyval_is_alt())
	{
		if(status)
			terminal->keyflags |= KEY_FLAG_ALT;
		else
			terminal->keyflags &= ~KEY_FLAG_ALT;
		v3270_draw_alt_status(terminal);
	}
#endif // KEY_FLAG_ALT

 }

 static gboolean check_keypress(v3270 *widget, const GdkEventKey *event)
 {
 	//GdkKeymap * keymap = gdk_keymap_get_for_display(gtk_widget_get_display(GTK_WIDGET(widget)));

 	// From gtk_accelerator_name at https://gitlab.gnome.org/GNOME/gtk/blob/master/gtk/gtkaccelgroup.c
 	// Side steps issue from https://mail.gnome.org/archives/gtk-app-devel-list/2007-August/msg00053.html
	guint keyval = gdk_keyval_to_lower(event->keyval);

	// Add virtual modifiers to event state.
	GdkModifierType state = event->state & GDK_MODIFIER_MASK;
	// gdk_keymap_add_virtual_modifiers(keymap,&state);

/*
#ifdef WIN32
	// FIXME (perry#1#): Find a better way!
	if( event->keyval == 0xffffff && event->hardware_keycode == 0x0013)
		keyval = GDK_Pause;

	// Windows sets <ctrl> in left/right control
	else if(state & GDK_CONTROL_MASK && (keyval == GDK_Control_R || keyval == GDK_Control_L))
		state &= ~GDK_CONTROL_MASK;
#endif
*/

 	// Check if the application can handle the key.
	gboolean handled = FALSE;
	g_signal_emit(
		GTK_WIDGET(widget),
		v3270_widget_signal[V3270_SIGNAL_KEYPRESS],
		0,
		keyval,
		event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK),	// FIXME: use the processed state after the main application update.
		&handled
	);

	//debug("Keyboard action was %s (keyval=%08x state=%08x)",handled ? "Handled" : "Not handled",event->keyval,event->state);
	if(handled)
		return TRUE;

	if(!gtk_accelerator_valid(keyval,state))
		return FALSE;

	//
	// Check for accelerator.
	//
	const V3270Accelerator * accel = v3270_get_accelerator(GTK_WIDGET(widget), keyval, state);
	if(accel)
	{
		debug("%s will fire",__FUNCTION__);
		v3270_accelerator_activate(accel,GTK_WIDGET(widget));
		return TRUE;
	}

	// Check PFKeys
	if(keyval >= GDK_F1 && keyval <= GDK_F12 && !(state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		int pfcode = (keyval - GDK_F1) + ((state & GDK_SHIFT_MASK) ? 13 : 1);

		debug("%s: PF%d will fire",__FUNCTION__,pfcode);

		if(pfcode > 0 && pfcode < 25)
		{
			if(lib3270_pfkey(widget->host,pfcode))
				gdk_display_beep(gtk_widget_get_display(GTK_WIDGET(widget)));
			return TRUE;
		}
		else
		{
			g_warning("Invalid PFCode %d",pfcode);
		}
	}

 	/*
	gboolean				  handled	= FALSE;
	const V3270Accelerator	* accel;

	g_signal_emit(
		GTK_WIDGET(widget),
		v3270_widget_signal[V3270_SIGNAL_KEYPRESS],
		0,
		event->keyval,
		event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK),
		&handled
	);
	debug("Keyboard action was %s",handled ? "Handled" : "Not handled");
	if(handled)
		return TRUE;

#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(event->keyval, event->state);
		debug("Keyname: %s",keyname);
	}
#endif // DEBUG

	// Check accelerator table.
	accel = v3270_get_accelerator(GTK_WIDGET(widget), event->keyval, event->state);

	if(!accel)
		accel = v3270_get_accelerator(GTK_WIDGET(widget), event->keyval, event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK));

	if(accel)
	{
		debug("%s will fire",__FUNCTION__);
		v3270_accelerator_activate(accel,GTK_WIDGET(widget));
		return TRUE;
	}

	// Check PFKeys
	if(event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		int pfcode = (event->keyval - GDK_F1) + ((event->state & GDK_SHIFT_MASK) ? 13 : 1);

		if(pfcode > 0 && pfcode < 25)
		{
			lib3270_pfkey(widget->host,pfcode);
			return TRUE;
		}
	}
	*/

 	return FALSE;
 }

 gboolean v3270_key_press_event(GtkWidget *widget, GdkEventKey *event)
 {
	v3270 * terminal = GTK_V3270(widget);

	terminal->activity.timestamp = time(0);
 	update_keyboard_state(terminal,event,TRUE);

	if(event->state & GDK_NUMLOCK_MASK)
	{
		// Hack for special keys
		const V3270Accelerator * acel = v3270_get_accelerator(widget, event->keyval, event->state);

		debug("acel=%p",acel);

		if(acel)
		{
			debug("%s will fire",__FUNCTION__);
			v3270_accelerator_activate(acel,GTK_WIDGET(widget));
			gtk_im_context_reset(terminal->input_method);
			return TRUE;
		}
	}

	if(gtk_im_context_filter_keypress(terminal->input_method,event))
		return TRUE;

	if(check_keypress(terminal,event))
	{
		gtk_im_context_reset(terminal->input_method);
		return TRUE;
	}

	return FALSE;

 }

 gboolean v3270_key_release_event(GtkWidget *widget, GdkEventKey *event)
 {
	v3270 * terminal = GTK_V3270(widget);

 	update_keyboard_state(terminal,event,FALSE);

	if(gtk_im_context_filter_keypress(terminal->input_method,event))
		return TRUE;

	return FALSE;

 }

 LIB3270_EXPORT void v3270_tab(GtkWidget *widget)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
	GTK_V3270(widget)->activity.timestamp = time(0);
	lib3270_nextfield(GTK_V3270(widget)->host);
 }

 LIB3270_EXPORT void v3270_backtab(GtkWidget *widget)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
	GTK_V3270(widget)->activity.timestamp = time(0);
	lib3270_previousfield(GTK_V3270(widget)->host);
 }

 LIB3270_EXPORT void v3270_set_string(GtkWidget *widget, const gchar *str)
 {
 	H3270 *host;
	gchar *utf;

	g_return_if_fail(GTK_IS_V3270(widget));

	host = GTK_V3270(widget)->host;
	GTK_V3270(widget)->activity.timestamp = time(0);

	utf = g_convert((char *) str, -1, lib3270_get_display_charset(host), "UTF-8", NULL, NULL, NULL);

	if(utf)
	{
		lib3270_set_string(host, (const unsigned char *) utf, -1);
		g_free(utf);
	}

 }

 void v3270_key_commit(G_GNUC_UNUSED GtkIMContext *imcontext, gchar *str, v3270 *widget)
 {
	gchar *utf = g_convert((char *) str, -1, lib3270_get_display_charset(widget->host), "UTF-8", NULL, NULL, NULL);

	if(utf)
	{
		lib3270_input_string(GTK_V3270(widget)->host, (const unsigned char *) utf, -1);
		g_free(utf);
	}
	else
	{
		lib3270_ring_bell(widget->host);
	}
 }


