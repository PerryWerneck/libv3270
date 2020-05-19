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

 #include <config.h>
 #include <terminal.h>
 #include "marshal.h"

 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/toggle.h>
 #include <lib3270/actions.h>
 #include <internals.h>

 #if GTK_CHECK_VERSION(3,0,0)
	#include <gdk/gdkkeysyms-compat.h>
 #else
	#include <gdk/gdkkeysyms.h>
 #endif

 #define WIDTH_IN_PIXELS(terminal,x) (x * cols)
 #define HEIGHT_IN_PIXELS(terminal,x) (x * (rows+1))

 #define CONTENTS_WIDTH(terminal) (cols * terminal->font.width)
 #define CONTENTS_HEIGHT(terminal) (((rows+1) * terminal->font.spacing)+OIA_TOP_MARGIN+2)

/**
 * SECTION:	v3270
 * @title:	Virtual 3270 widget
 * @short_description:	The virtual 3270 terminal widget.
 *
 * Common functions for interact with the 3270 virtual terminal widget.
 *
 */

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 G_DEFINE_TYPE(v3270, v3270, GTK_TYPE_WIDGET);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

static guint v3270_widget_signal[V3270_SIGNAL_LAST] = { 0 };

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void			  v3270_realize				(	GtkWidget		* widget) ;
static void			  v3270_size_allocate		(	GtkWidget		* widget,
													GtkAllocation	* allocation );
static gboolean		  v3270_focus_in_event(GtkWidget *widget, GdkEventFocus *event);
static gboolean		  v3270_focus_out_event(GtkWidget *widget, GdkEventFocus *event);

static void			  v3270_destroy		(GtkWidget		* object);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static gboolean v3270_popup_menu(GtkWidget * widget)
{
	GdkEventButton event;

	memset(&event,0,sizeof(event));

	event.time	 = gtk_get_current_event_time();
	event.button = 3;
	event.type 	 = GDK_BUTTON_PRESS;

	v3270_emit_popup(	GTK_V3270(widget),
						lib3270_get_cursor_address(GTK_V3270(widget)->host),
						&event );

	return TRUE;
}

void get_preferred_height(GtkWidget *widget, gint *minimum_height, gint *natural_height)
{
	int height = GTK_V3270(widget)->minimum_height;

	if(minimum_height)
		*minimum_height = height ? height : 10;

	if(natural_height)
		*natural_height = 400;

}

void get_preferred_width(GtkWidget *widget, gint *minimum_width, gint *natural_width)
{
	int width = GTK_V3270(widget)->minimum_width;

	if(minimum_width)
		*minimum_width = width ? width : 10;

	if(natural_width)
		*natural_width = 600;
}

gboolean v3270_query_tooltip(GtkWidget  *widget, gint x, gint y, G_GNUC_UNUSED gboolean keyboard_tooltip, GtkTooltip *tooltip)
{
	if(y >= GTK_V3270(widget)->oia.rect->y)
	{
		GdkRectangle *rect = GTK_V3270(widget)->oia.rect;

		if(x >= rect[V3270_OIA_SSL].x && x <= (rect[V3270_OIA_SSL].x + rect[V3270_OIA_SSL].width))
		{
			H3270 *hSession = GTK_V3270(widget)->host;

			if(!lib3270_is_connected(hSession))
			{
#ifndef _WIN32
				gtk_tooltip_set_icon_from_icon_name(tooltip,"gtk-disconnect",GTK_ICON_SIZE_DIALOG);
#endif // GTK_CHECK_VERSION
				gtk_tooltip_set_markup(tooltip,_( "<b>Identity not verified</b>\nDisconnected from host" ) );
			}
			else
			{
#ifndef _WIN32
				gtk_tooltip_set_icon_from_icon_name(tooltip,lib3270_get_ssl_state_icon_name(hSession),GTK_ICON_SIZE_DIALOG);
#endif
				g_autofree gchar * message =
						g_strdup_printf(
							"<b>%s</b>\n%s",
								lib3270_get_ssl_state_message(hSession),
								lib3270_get_ssl_state_description(hSession)
						);
				gtk_tooltip_set_markup(tooltip,message);

			}

			return TRUE;
		}

	}
	return FALSE;
}

static void v3270_toggle_changed(G_GNUC_UNUSED v3270 *widget, G_GNUC_UNUSED LIB3270_TOGGLE_ID toggle_id, G_GNUC_UNUSED gboolean toggle_state, G_GNUC_UNUSED const gchar *toggle_name)
{
}

static void finalize(GObject *object) {

	debug("V3270::%s",__FUNCTION__);

	v3270 * terminal = GTK_V3270(object);

	if(terminal->remap_filename) {
		g_free(terminal->remap_filename);
		terminal->remap_filename = NULL;
	}

	if(terminal->host) {
		// Release session
		lib3270_session_free(terminal->host);
		terminal->host = NULL;
	}

	if(terminal->accelerators) {
		g_slist_free_full(terminal->accelerators,g_free);
		terminal->accelerators = NULL;
	}

	if(terminal->selection.font_family) {
		g_free(terminal->selection.font_family);
		terminal->selection.font_family = NULL;
	}

	if(terminal->selection.color.scheme) {
		g_free(terminal->selection.color.scheme);
		terminal->selection.color.scheme = NULL;
	}

	if(terminal->selection.color.value) {
		g_free(terminal->selection.color.value);
		terminal->selection.color.value = NULL;
	}

	G_OBJECT_CLASS(v3270_parent_class)->finalize(object);
 }

static void v3270_class_init(v3270Class *klass)
{
	GObjectClass	* gobject_class	= G_OBJECT_CLASS(klass);
	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);
	GtkBindingSet	* binding		= gtk_binding_set_by_class(klass);

	// Setup widget key bindings
	gtk_binding_entry_skip(binding,GDK_F10,0);

	// Object methods
	gobject_class->finalize							= finalize;

	// Atoms
	klass->clipboard_formatted						= gdk_atom_intern_static_string("application/x-v3270-formatted");

	// Widget methods
	widget_class->realize 							= v3270_realize;
	widget_class->size_allocate						= v3270_size_allocate;
	widget_class->key_press_event					= v3270_key_press_event;
	widget_class->key_release_event					= v3270_key_release_event;
	widget_class->focus_in_event					= v3270_focus_in_event;
	widget_class->focus_out_event					= v3270_focus_out_event;
	widget_class->button_press_event				= v3270_button_press_event;
	widget_class->button_release_event				= v3270_button_release_event;
	widget_class->motion_notify_event				= v3270_motion_notify_event;
	widget_class->popup_menu						= v3270_popup_menu;
	widget_class->scroll_event						= v3270_scroll_event;
	widget_class->query_tooltip						= v3270_query_tooltip;

	widget_class->get_preferred_height				= get_preferred_height;
	widget_class->get_preferred_width				= get_preferred_width;

	widget_class->destroy 							= v3270_destroy;
	widget_class->draw 								= v3270_draw;

	// Accessibility support
	widget_class->get_accessible 					= v3270_get_accessible;

	klass->activate									= v3270_activate;
	klass->toggle_changed 							= v3270_toggle_changed;
	klass->message_changed 							= v3270_update_message;
	klass->popup_message							= v3270_popup_message;

	// Register I/O Handlers
	v3270_register_io_handlers(klass);

	// Cursors
	{
#ifdef WIN32
		// http://git.gnome.org/browse/gtk+/tree/gdk/win32/gdkcursor-win32.c
		// http://www.functionx.com/win32/Lesson02b.htm
		static const gchar	* cr[LIB3270_POINTER_COUNT] =
		{
			"ibeam",	//	LIB3270_POINTER_UNLOCKED
			"wait",		//	LIB3270_POINTER_WAITING
			"arrow",	//	LIB3270_POINTER_LOCKED
			"arrow",	//	LIB3270_POINTER_PROTECTED
			"hand",		//	LIB3270_POINTER_MOVE_SELECTION
			"sizenwse",	//	LIB3270_POINTER_SELECTION_TOP_LEFT
			"sizenesw",	//	LIB3270_POINTER_SELECTION_TOP_RIGHT
			"sizens",	//	LIB3270_POINTER_SELECTION_TOP
			"sizenesw",	//	LIB3270_POINTER_SELECTION_BOTTOM_LEFT
			"sizenwse",	//	LIB3270_POINTER_SELECTION_BOTTOM_RIGHT
			"sizens",	//	LIB3270_POINTER_SELECTION_BOTTOM
			"sizewe",	//	LIB3270_POINTER_SELECTION_LEFT
			"sizewe",	//	LIB3270_POINTER_SELECTION_RIGHT
			"help",		//	LIB3270_POINTER_QUESTION
		};
#else
		static const int	  cr[LIB3270_POINTER_COUNT] =
		{
			GDK_XTERM,					// LIB3270_POINTER_UNLOCKED
			GDK_WATCH,					// LIB3270_POINTER_WAITING
			GDK_X_CURSOR,				// LIB3270_POINTER_LOCKED
			GDK_ARROW,					// LIB3270_POINTER_PROTECTED
			GDK_HAND1,					// LIB3270_POINTER_MOVE_SELECTION
			GDK_TOP_LEFT_CORNER, 		// LIB3270_POINTER_SELECTION_TOP_LEFT
			GDK_TOP_RIGHT_CORNER,		// LIB3270_POINTER_SELECTION_TOP_RIGHT
			GDK_TOP_SIDE,				// LIB3270_POINTER_SELECTION_TOP
			GDK_BOTTOM_LEFT_CORNER,		// LIB3270_POINTER_SELECTION_BOTTOM_LEFT
			GDK_BOTTOM_RIGHT_CORNER,	// LIB3270_POINTER_SELECTION_BOTTOM_RIGHT
			GDK_BOTTOM_SIDE,			// LIB3270_POINTER_SELECTION_BOTTOM
			GDK_LEFT_SIDE,				// LIB3270_POINTER_SELECTION_LEFT
			GDK_RIGHT_SIDE,				// LIB3270_POINTER_SELECTION_RIGHT
			GDK_QUESTION_ARROW,			// LIB3270_POINTER_QUESTION
		};
#endif // WIN32

		int f;

		for(f=0;f<LIB3270_POINTER_COUNT;f++)
		{
#ifdef WIN32
			klass->cursors[f] = gdk_cursor_new_from_name(gdk_display_get_default(),cr[f]);
#else
			klass->cursors[f] = gdk_cursor_new_for_display(gdk_display_get_default(),cr[f]);
#endif
		}
	}

	// Signals
	widget_class->activate_signal =
		g_signal_new(	I_("activate"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET (v3270Class, activate),
						NULL, NULL,
						v3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[V3270_SIGNAL_TOGGLE_CHANGED] =
		g_signal_new(	I_("toggle_changed"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, toggle_changed),
						NULL, NULL,
						v3270_VOID__VOID_ENUM_BOOLEAN_POINTER,
						G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_STRING);

	v3270_widget_signal[V3270_SIGNAL_MESSAGE_CHANGED] =
		g_signal_new(	I_("message_changed"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, message_changed),
						NULL, NULL,
						v3270_VOID__VOID_ENUM,
						G_TYPE_NONE, 1, G_TYPE_UINT);

	v3270_widget_signal[V3270_SIGNAL_KEYPRESS] =
		g_signal_new(	I_("keypress"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__UINT_ENUM,
						G_TYPE_BOOLEAN, 2, G_TYPE_UINT, G_TYPE_UINT);

	v3270_widget_signal[V3270_SIGNAL_CONNECTED] =
		g_signal_new(	I_("connected"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__POINTER,
						G_TYPE_NONE, 1, G_TYPE_STRING);

	v3270_widget_signal[V3270_SIGNAL_DISCONNECTED] =
		g_signal_new(	I_("disconnected"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[V3270_SIGNAL_SESSION_CHANGED] =
		g_signal_new(	I_("session_changed"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[V3270_SIGNAL_MODEL_CHANGED] =
		g_signal_new(	I_("model_changed"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_UINT_POINTER,
						G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

	v3270_widget_signal[V3270_SIGNAL_SELECTING] =
		g_signal_new(	I_("selecting"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[V3270_SIGNAL_POPUP] =
		g_signal_new(	I_("popup"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__VOID_BOOLEAN_BOOLEAN_POINTER,
						G_TYPE_BOOLEAN, 3, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_POINTER);

	v3270_widget_signal[V3270_SIGNAL_OIA_POPUP] =
		g_signal_new(	I_("oia-popup"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__VOID_UINT_POINTER,
						G_TYPE_BOOLEAN, 2, G_TYPE_UINT, G_TYPE_POINTER);

	v3270_widget_signal[V3270_SIGNAL_PASTENEXT] =
		g_signal_new(	I_("pastenext"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[V3270_SIGNAL_CLIPBOARD] =
		g_signal_new(	I_("has_text"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[V3270_SIGNAL_CHANGED] =
		g_signal_new(	I_("changed"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_UINT_UINT,
						G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

	v3270_widget_signal[V3270_SIGNAL_MESSAGE] =
		g_signal_new(	I_("popup_message"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, popup_message),
						NULL, NULL,
						v3270_VOID__VOID_UINT_POINTER_POINTER_POINTER,
						G_TYPE_NONE, 4, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	v3270_widget_signal[V3270_SIGNAL_FIELD] =
		g_signal_new(	I_("field_clicked"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__VOID_BOOLEAN_UINT_POINTER,
						G_TYPE_BOOLEAN, 3, G_TYPE_BOOLEAN, G_TYPE_UINT, G_TYPE_POINTER);


	v3270_widget_signal[V3270_SIGNAL_PRINT_DONE] =
		g_signal_new(	I_("print-done"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_POINTER_UINT,
						G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_UINT, 0);

	v3270_widget_signal[V3270_SIGNAL_PRINT_SETUP] =
		g_signal_new(	I_("print-setup"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_POINTER,
						G_TYPE_NONE, 1, G_TYPE_POINTER, 0);

	v3270_widget_signal[V3270_SIGNAL_SAVE_SETTINGS] =
		g_signal_new(	I_("save-settings"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_init_properties(gobject_class);

}

static gboolean activity_tick(v3270 *widget)
{
	if(widget->activity.disconnect && lib3270_is_connected(widget->host) && ((guint) ((time(0) - widget->activity.timestamp)/60)) >= widget->activity.disconnect)
		lib3270_disconnect(widget->host);
	return TRUE;
}

static void release_activity_timer(v3270 *widget)
{
	widget->activity.timer = NULL;
}

static void v3270_init(v3270 *widget)
{
	size_t ix;

	widget->host = lib3270_session_new(NULL);
	lib3270_set_user_data(widget->host,widget);
	lib3270_set_url(widget->host,NULL);

	// Install callbacks
	v3270_install_callbacks(widget);

	// Setup clipboard.
	widget->selection.target		= GDK_SELECTION_CLIPBOARD;
	widget->selection.options		= V3270_SELECTION_DEFAULT;

	// Reset timer
	widget->activity.timestamp		= time(0);
	widget->activity.disconnect		= 0;

	// Setup input method
	widget->input_method 			= gtk_im_multicontext_new();
    g_signal_connect(G_OBJECT(widget->input_method),"commit",G_CALLBACK(v3270_key_commit),widget);

#if GTK_CHECK_VERSION(2,18,0)
	gtk_widget_set_can_default(GTK_WIDGET(widget),TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(widget),TRUE);
#else
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(widget),(GTK_CAN_DEFAULT|GTK_CAN_FOCUS));
#endif // GTK(2,18)

	// Setup widget
    gtk_widget_add_events(GTK_WIDGET(widget),GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_ENTER_NOTIFY_MASK|GDK_SCROLL_MASK);
	gtk_widget_set_has_tooltip(GTK_WIDGET(widget),TRUE);

	// Setup auto disconnect timer
	widget->cursor.timer = NULL;

	// Enable drawing
	widget->drawing	= 1;

	// Set defaults
	v3270_font_info_init(&widget->font);
	v3270_set_color_table(widget->color,v3270_get_default_colors());

	for(ix = 0; ix < G_N_ELEMENTS(widget->responses); ix++)
		widget->responses[ix] = GTK_RESPONSE_NONE;

	// Init accelerators
	v3270_init_accelerators(widget);

}

LIB3270_EXPORT GtkWidget * v3270_new(void)
{
	return g_object_new(GTK_TYPE_V3270, NULL);
}

static void v3270_destroy(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

	debug("%s",__FUNCTION__);

	if(terminal->host)
	{
		// Cleanup
		lib3270_reset_callbacks(terminal->host);
		lib3270_set_user_data(terminal->host,NULL);
	}

	if(terminal->accessible)
	{
		gtk_accessible_set_widget(terminal->accessible, NULL);
		g_object_unref(terminal->accessible);
		terminal->accessible = NULL;
	}

	v3270_font_info_unset(&terminal->font);

	if(terminal->surface)
	{
		cairo_surface_destroy(terminal->surface);
		terminal->surface = NULL;
	}

	if(terminal->cursor.surface)
	{
		cairo_surface_destroy(terminal->cursor.surface);
		terminal->cursor.surface = NULL;
	}

	if(terminal->timer)
	{
		g_source_destroy(terminal->timer);
		while(terminal->timer)
			g_source_unref(terminal->timer);
	}

	if(terminal->blink.timer)
	{
		g_source_destroy(terminal->blink.timer);
		while(terminal->blink.timer)
			g_source_unref(terminal->blink.timer);
	}

	if(terminal->cursor.timer)
	{
		g_source_destroy(terminal->cursor.timer);
		while(terminal->cursor.timer)
			g_source_unref(terminal->cursor.timer);
	}

	if(terminal->activity.timer)
	{
		g_source_destroy(terminal->activity.timer);
		while(terminal->activity.timer)
			g_source_unref(terminal->activity.timer);
	}

	if(terminal->input_method)
	{
		g_object_unref(terminal->input_method);
		terminal->input_method = NULL;
	}

    v3270_clear_selection(terminal);

	if(terminal->session.name)
	{
		g_free(terminal->session.name);
		terminal->session.name = NULL;
	}

	if(terminal->session.title)
	{
		g_free(terminal->session.title);
		terminal->session.title = NULL;
	}

	GTK_WIDGET_CLASS(v3270_parent_class)->destroy(widget);

}

static gboolean timer_tick(v3270 *widget)
{
	if(lib3270_get_toggle(widget->host,LIB3270_TOGGLE_CURSOR_BLINK))
	{
		widget->cursor.show ^= 1;
		v3270_queue_draw_area(GTK_WIDGET(widget),	widget->cursor.rect.x,
													widget->cursor.rect.y,
													widget->cursor.rect.width,
													widget->cursor.rect.height );
	}

	return TRUE;
}

static void release_cursor_timer(v3270 *widget)
{
	widget->cursor.timer = NULL;
}

static gboolean bg_pos_realize(v3270 *terminal)
{
	if(lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_FULL_SCREEN))
		gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(terminal))));

	if(lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_CONNECT_ON_STARTUP) && lib3270_is_disconnected(terminal->host))
		v3270_reconnect(GTK_WIDGET(terminal));

	return FALSE;
}

static void v3270_realize(GtkWidget	* widget)
{
	if(!gtk_widget_get_has_window(widget))
	{
		GTK_WIDGET_CLASS(v3270_parent_class)->realize(widget);
	}
	else
	{
		GtkAllocation allocation;
		GdkWindow *window;
		GdkWindowAttr attributes;
		gint attributes_mask;

		gtk_widget_set_realized (widget, TRUE);

		gtk_widget_get_allocation (widget, &allocation);

		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.x = allocation.x;
		attributes.y = allocation.y;
		attributes.width = allocation.width;
		attributes.height = allocation.height;
		attributes.wclass = GDK_INPUT_OUTPUT;
		attributes.visual = gtk_widget_get_visual (widget);
		attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

		attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

		window = gdk_window_new(gtk_widget_get_parent_window (widget),&attributes, attributes_mask);
		gdk_window_set_user_data (window, widget);
		gtk_widget_set_window(widget, window);

		gtk_im_context_set_client_window(GTK_V3270(widget)->input_method,window);

		g_idle_add((GSourceFunc) bg_pos_realize, GTK_V3270(widget));

	}

	v3270_reconfigure(GTK_V3270(widget));

	if(!GTK_V3270(widget)->cursor.timer)
	{
		// Setup cursor blink timer
		v3270 *terminal = GTK_V3270(widget);

		terminal->cursor.timer = g_timeout_source_new(500);
		g_source_set_callback(terminal->cursor.timer,(GSourceFunc) timer_tick, widget, (GDestroyNotify) release_cursor_timer);

		g_source_attach(terminal->cursor.timer, NULL);
		g_source_unref(terminal->cursor.timer);
	}

	if(!GTK_V3270(widget)->activity.timer)
	{
		v3270 *terminal = GTK_V3270(widget);

		terminal->activity.timer = g_timeout_source_new(10000);
		g_source_set_callback(terminal->activity.timer,(GSourceFunc) activity_tick, widget, (GDestroyNotify) release_activity_timer);
		g_source_attach(terminal->activity.timer, NULL);
		g_source_unref(terminal->activity.timer);
	}

}

static void v3270_size_allocate(GtkWidget * widget, GtkAllocation * allocation)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	g_return_if_fail(allocation != NULL);

	gtk_widget_set_allocation(widget, allocation);

	if(gtk_widget_get_realized(widget))
	{
		if(gtk_widget_get_has_window(widget))
			gdk_window_move_resize(gtk_widget_get_window (widget),allocation->x, allocation->y,allocation->width, allocation->height);

		v3270_reconfigure(GTK_V3270(widget));

	}
}

#if ! GTK_CHECK_VERSION(2,18,0)
G_GNUC_INTERNAL void gtk_widget_get_allocation(GtkWidget *widget, GtkAllocation *allocation)
{
	*allocation = widget->allocation;
}
#endif // !GTK(2,18)


static gboolean notify_focus(GtkWidget *widget, GdkEventFocus *event)
{
	GtkAccessible *obj = GTK_V3270(widget)->accessible;

	if(obj)
		g_signal_emit_by_name (obj, "focus-event", event->in);

	return FALSE;
}

gboolean v3270_focus_in_event(GtkWidget *widget, GdkEventFocus *event)
{
	v3270 * terminal = GTK_V3270(widget);

	gtk_im_context_focus_in(terminal->input_method);

	return notify_focus(widget,event);
}

gboolean v3270_focus_out_event(GtkWidget *widget, GdkEventFocus *event)
{
	v3270 * terminal = GTK_V3270(widget);

	gtk_im_context_focus_out(terminal->input_method);

	return notify_focus(widget,event);
}

const GtkWidgetClass * v3270_get_parent_class(void)
{
	return GTK_WIDGET_CLASS(v3270_parent_class);
}

LIB3270_EXPORT GtkIMContext * v3270_get_im_context(GtkWidget *widget)
{
	return GTK_V3270(widget)->input_method;
}

static gboolean bg_emit_save_settings(v3270 *terminal)
{
	GdkWindow * window = gtk_widget_get_window(GTK_WIDGET(terminal));
	gdk_window_set_cursor(
		window,
		GTK_V3270_GET_CLASS(terminal)->cursors[LIB3270_POINTER_WAITING]
	);

	gdk_display_sync(gtk_widget_get_display(GTK_WIDGET(terminal)));

	terminal->freeze = 0;
	g_signal_emit(terminal,v3270_widget_signal[V3270_SIGNAL_SAVE_SETTINGS], 0, FALSE);

	gdk_window_set_cursor(
		window,
		NULL
	);

	g_object_unref(terminal);

 	return FALSE;
}

void v3270_emit_save_settings(GtkWidget *widget)
{
	debug("%s(Freeze is %s)",__FUNCTION__,GTK_V3270(widget)->freeze ? "ON" : "OFF");
	if(widget && GTK_IS_V3270(widget) && !GTK_V3270(widget)->freeze)
	{
		g_object_ref(widget);
		GTK_V3270(widget)->freeze = 1;
		g_idle_add((GSourceFunc) bg_emit_save_settings, G_OBJECT(widget));
	}
}

void v3270_signal_emit(gpointer instance, enum V3270_SIGNAL signal_id, ...)
{
  va_list var_args;

  va_start (var_args, signal_id);
  g_signal_emit_valist(instance, v3270_widget_signal[signal_id], 0, var_args);
  va_end (var_args);

}


