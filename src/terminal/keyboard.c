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

 gboolean v3270_key_press_event(GtkWidget *widget, GdkEventKey *event)
 {
	v3270 * terminal = GTK_V3270(widget);

	terminal->activity.timestamp = time(0);
 	update_keyboard_state(terminal,event,TRUE);

#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(event->keyval,event->state);
		debug("%s Keyval: %d (%s) State: %04x %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
				__FUNCTION__,
				event->keyval,
				gdk_keyval_name(event->keyval),
				event->state,
				event->state & GDK_SHIFT_MASK		? " GDK_SHIFT_MASK"		: "",
				event->state & GDK_LOCK_MASK		? " GDK_LOCK_MASK"		: "",
				event->state & GDK_CONTROL_MASK		? " GDK_CONTROL_MASK"	: "",
				event->state & GDK_MOD1_MASK		? " GDK_MOD1_MASK"		: "",
				event->state & GDK_MOD2_MASK		? " GDK_MOD2_MASK"		: "",
				event->state & GDK_MOD3_MASK		? " GDK_MOD3_MASK"		: "",
				event->state & GDK_MOD4_MASK		? " GDK_MOD4_MASK"		: "",
				event->state & GDK_MOD5_MASK		? " GDK_MOD5_MASK"		: "",
				event->state & GDK_BUTTON1_MASK		? " GDK_BUTTON1_MASK"	: "",
				event->state & GDK_BUTTON2_MASK		? " GDK_BUTTON2_MASK"	: "",
				event->state & GDK_BUTTON3_MASK		? " GDK_BUTTON3_MASK"	: "",
				event->state & GDK_BUTTON4_MASK		? " GDK_BUTTON4_MASK"	: "",
				event->state & GDK_BUTTON5_MASK		? " GDK_BUTTON5_MASK"	: "",
				event->state & GDK_RELEASE_MASK		? " GDK_RELEASE_MASK"	: "",
				event->state & GDK_MODIFIER_MASK	? " GDK_MODIFIER_MASK"	: ""
			);

	}
#endif // DEBUG

	if(gtk_im_context_filter_keypress(terminal->input_method,event))
		return TRUE;

	/*
	if(!gtk_accelerator_valid(event->keyval,event->state))
		return FALSE;
	*/

	// Signal to the application.
	gboolean handled = FALSE;
	v3270_signal_emit(
		GTK_WIDGET(widget),
		V3270_SIGNAL_KEYPRESS,
		event->keyval,
		event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK),
		&handled
	);

#ifdef DEBUG
	{
		g_autofree gchar * key_name = gtk_accelerator_name(event->keyval,event->state);
		debug("Keyboard action \"%s\" was %s",key_name,handled ? "Handled" : "Not handled");
	}
#endif // DEBUG
	if(handled)
		return TRUE;


	// Check for s
	const V3270Accelerator * accelerator = v3270_accelerator_map_lookup_entry(widget, event->keyval, event->state);
	if(accelerator)
	{
		debug("Found accelerator %s",v3270_accelerator_get_name(accelerator));
		v3270_accelerator_activate(accelerator,widget);
		return TRUE;
	}

	if(event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_CONTROL_MASK|GDK_MOD1_MASK)))
	{
		// It's a PFKey Action.
		int pfcode = (event->keyval - GDK_F1) + ((event->state & GDK_SHIFT_MASK) ? 13 : 1);

		if(pfcode > 0 && pfcode < 25)
		{
			if(lib3270_pfkey(GTK_V3270(widget)->host,pfcode))
				gdk_display_beep(gtk_widget_get_display(widget));

			return TRUE;
		}

	}

	return FALSE;

 }

 gboolean v3270_key_release_event(GtkWidget *widget, GdkEventKey *event)
 {
	v3270 * terminal = GTK_V3270(widget);

	terminal->activity.timestamp = time(0);
 	update_keyboard_state(terminal,event,FALSE);

/*
#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(event->keyval,event->state);
		debug("%s Keyval: %d (%s) State: %04x %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
				__FUNCTION__,
				event->keyval,
				gdk_keyval_name(event->keyval),
				event->state,
				event->state & GDK_SHIFT_MASK		? " GDK_SHIFT_MASK"		: "",
				event->state & GDK_LOCK_MASK		? " GDK_LOCK_MASK"		: "",
				event->state & GDK_CONTROL_MASK		? " GDK_CONTROL_MASK"	: "",
				event->state & GDK_MOD1_MASK		? " GDK_MOD1_MASK"		: "",
				event->state & GDK_MOD2_MASK		? " GDK_MOD2_MASK"		: "",
				event->state & GDK_MOD3_MASK		? " GDK_MOD3_MASK"		: "",
				event->state & GDK_MOD4_MASK		? " GDK_MOD4_MASK"		: "",
				event->state & GDK_MOD5_MASK		? " GDK_MOD5_MASK"		: "",
				event->state & GDK_BUTTON1_MASK		? " GDK_BUTTON1_MASK"	: "",
				event->state & GDK_BUTTON2_MASK		? " GDK_BUTTON2_MASK"	: "",
				event->state & GDK_BUTTON3_MASK		? " GDK_BUTTON3_MASK"	: "",
				event->state & GDK_BUTTON4_MASK		? " GDK_BUTTON4_MASK"	: "",
				event->state & GDK_BUTTON5_MASK		? " GDK_BUTTON5_MASK"	: "",
				event->state & GDK_RELEASE_MASK		? " GDK_RELEASE_MASK"	: "",
				event->state & GDK_MODIFIER_MASK	? " GDK_MODIFIER_MASK"	: ""
			);

	}
#endif // DEBUG
*/

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


