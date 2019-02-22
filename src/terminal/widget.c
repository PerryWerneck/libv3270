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
 #include "private.h"
 #include "marshal.h"

 #include <lib3270.h>
 #include <lib3270/log.h>
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

 guint		  		  v3270_widget_signal[LAST_SIGNAL]		= { 0 };
 GdkCursor			* v3270_cursor[LIB3270_POINTER_COUNT]	= { 0 };

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void			  v3270_realize				(	GtkWidget		* widget) ;
static void			  v3270_size_allocate		(	GtkWidget		* widget,
													GtkAllocation	* allocation );
static void			  v3270_send_configure		(	v3270			* terminal );

// Signals
static void v3270_activate			(GtkWidget *widget);

gboolean v3270_focus_in_event(GtkWidget *widget, GdkEventFocus *event);
gboolean v3270_focus_out_event(GtkWidget *widget, GdkEventFocus *event);

static void 	v3270_destroy		(GtkWidget		* object);

/*--[ Implement ]------------------------------------------------------------------------------------*/

void v3270_cursor_draw(v3270 *widget)
{
	int 			pos = lib3270_get_cursor_address(widget->host);
	unsigned char	c;
	unsigned short	attr;

	lib3270_get_contents(widget->host,pos,pos,&c,&attr);
	v3270_update_cursor_surface(widget,c,attr);
	v3270_queue_draw_area(	GTK_WIDGET(widget),
							widget->cursor.rect.x,widget->cursor.rect.y,
							widget->cursor.rect.width,widget->cursor.rect.height);

}

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

void v3270_popup_message(GtkWidget *widget, LIB3270_NOTIFY type , const gchar *title, const gchar *message, const gchar *text)
{
	GtkWidget		* dialog;
	GtkWidget		* toplevel	= NULL;
	GtkMessageType	  msgtype	= GTK_MESSAGE_WARNING;
	GtkButtonsType	  buttons	= GTK_BUTTONS_OK;

	if(widget && GTK_IS_WIDGET(widget))
		toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));

	if(!GTK_IS_WINDOW(toplevel))
		toplevel = NULL;

	if(type == LIB3270_NOTIFY_CRITICAL)
	{
		msgtype	= GTK_MESSAGE_ERROR;
		buttons = GTK_BUTTONS_CLOSE;
	}

	if(!title)
		title = _( "Error" );

	if(message)
	{
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,msgtype,buttons,"%s",message);
		if(text && *text)
			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",text);
	}
	else if(text && *text)
	{
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,msgtype,buttons,"%s",text);
	}
	else
	{
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,msgtype,buttons,"%s",title);
	}

	gtk_window_set_title(GTK_WINDOW(dialog),title);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

}

gboolean v3270_query_tooltip(GtkWidget  *widget, gint x, gint y, G_GNUC_UNUSED gboolean keyboard_tooltip, GtkTooltip *tooltip)
{
	if(y >= GTK_V3270(widget)->oia.rect->y)
	{
		GdkRectangle *rect = GTK_V3270(widget)->oia.rect;

		if(x >= rect[V3270_OIA_SSL].x && x <= (rect[V3270_OIA_SSL].x + rect[V3270_OIA_SSL].width))
		{
			H3270 *hSession = GTK_V3270(widget)->host;

			if(!lib3270_connected(hSession))
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

static void loghandler(G_GNUC_UNUSED H3270 *session, const char *module, int rc, const char *fmt, va_list args)
{
	g_logv(module,rc ? G_LOG_LEVEL_WARNING : G_LOG_LEVEL_MESSAGE, fmt, args);
}

static void v3270_toggle_changed(G_GNUC_UNUSED v3270 *widget, G_GNUC_UNUSED LIB3270_TOGGLE toggle_id, G_GNUC_UNUSED gboolean toggle_state, G_GNUC_UNUSED const gchar *toggle_name)
{
}

static void finalize(GObject *object)
 {
	debug("V3270::%s",__FUNCTION__);
	G_OBJECT_CLASS(v3270_parent_class)->finalize(object);
 }

static void v3270_class_init(v3270Class *klass)
{
	GObjectClass	* gobject_class	= G_OBJECT_CLASS(klass);
	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);
	GtkBindingSet	* binding		= gtk_binding_set_by_class(klass);

	// Setup widget key bindings
	gtk_binding_entry_skip(binding,GDK_F10,0);

	lib3270_set_log_handler(loghandler);

	// Object methods
	gobject_class->finalize							= finalize;

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
			"ibeam",	//	V3270_CURSOR_UNPROTECTED
			"wait",		//	V3270_CURSOR_WAITING
			"arrow",	//	V3270_CURSOR_LOCKED
			"arrow",	//	V3270_CURSOR_PROTECTED
			"hand",		//	V3270_CURSOR_MOVE_SELECTION
			"sizenwse",	//	V3270_CURSOR_SELECTION_TOP_LEFT
			"sizenesw",	//	V3270_CURSOR_SELECTION_TOP_RIGHT
			"sizens",	//	V3270_CURSOR_SELECTION_TOP
			"sizenesw",	//	V3270_CURSOR_SELECTION_BOTTOM_LEFT
			"sizenwse",	//	V3270_CURSOR_SELECTION_BOTTOM_RIGHT
			"sizens",	//	V3270_CURSOR_SELECTION_BOTTOM
			"sizewe",	//	V3270_CURSOR_SELECTION_LEFT
			"sizewe",	//	V3270_CURSOR_SELECTION_RIGHT
			"help",		//	V3270_CURSOR_QUESTION
		};
#else
		static const int	  cr[LIB3270_POINTER_COUNT] =
		{
			GDK_XTERM,					// V3270_CURSOR_UNPROTECTED
			GDK_WATCH,					// V3270_CURSOR_WAITING
			GDK_X_CURSOR,				// V3270_CURSOR_LOCKED
			GDK_ARROW,					// V3270_CURSOR_PROTECTED
			GDK_HAND1,					// V3270_CURSOR_MOVE_SELECTION
			GDK_TOP_LEFT_CORNER, 		// V3270_CURSOR_SELECTION_TOP_LEFT
			GDK_TOP_RIGHT_CORNER,		// V3270_CURSOR_SELECTION_TOP_RIGHT
			GDK_TOP_SIDE,				// V3270_CURSOR_SELECTION_TOP
			GDK_BOTTOM_LEFT_CORNER,		// V3270_CURSOR_SELECTION_BOTTOM_LEFT
			GDK_BOTTOM_RIGHT_CORNER,	// V3270_CURSOR_SELECTION_BOTTOM_RIGHT
			GDK_BOTTOM_SIDE,			// V3270_CURSOR_SELECTION_BOTTOM
			GDK_LEFT_SIDE,				// V3270_CURSOR_SELECTION_LEFT
			GDK_RIGHT_SIDE,				// V3270_CURSOR_SELECTION_RIGHT
			GDK_QUESTION_ARROW,			// V3270_CURSOR_QUESTION
		};
#endif // WIN32

		int f;

		for(f=0;f<LIB3270_POINTER_COUNT;f++)
		{
#ifdef WIN32
			v3270_cursor[f] = gdk_cursor_new_from_name(gdk_display_get_default(),cr[f]);
#else
			v3270_cursor[f] = gdk_cursor_new_for_display(gdk_display_get_default(),cr[f]);
#endif
		}
	}

	// Signals
	widget_class->activate_signal =
		g_signal_new(	"activate",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET (v3270Class, activate),
						NULL, NULL,
						v3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[SIGNAL_TOGGLE_CHANGED] =
		g_signal_new(	"toggle_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, toggle_changed),
						NULL, NULL,
						v3270_VOID__VOID_ENUM_BOOLEAN_POINTER,
						G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_MESSAGE_CHANGED] =
		g_signal_new(	"message_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, message_changed),
						NULL, NULL,
						v3270_VOID__VOID_ENUM,
						G_TYPE_NONE, 1, G_TYPE_UINT);

	v3270_widget_signal[SIGNAL_KEYPRESS] =
		g_signal_new(	"keypress",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__UINT_ENUM,
						G_TYPE_BOOLEAN, 2, G_TYPE_UINT, G_TYPE_UINT);

	v3270_widget_signal[SIGNAL_CONNECTED] =
		g_signal_new(	"connected",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__POINTER,
						G_TYPE_NONE, 1, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_DISCONNECTED] =
		g_signal_new(	"disconnected",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[SIGNAL_UPDATE_CONFIG] =
		g_signal_new(	"update_config",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_POINTER_POINTER,
						G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_MODEL_CHANGED] =
		g_signal_new(	"model_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_UINT_POINTER,
						G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_SELECTING] =
		g_signal_new(	"selecting",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[SIGNAL_POPUP] =
		g_signal_new(	"popup",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__VOID_BOOLEAN_BOOLEAN_POINTER,
						G_TYPE_BOOLEAN, 3, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_POINTER);

	v3270_widget_signal[SIGNAL_PASTENEXT] =
		g_signal_new(	"pastenext",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[SIGNAL_CLIPBOARD] =
		g_signal_new(	"has_text",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[SIGNAL_CHANGED] =
		g_signal_new(	"changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_UINT_UINT,
						G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

	v3270_widget_signal[SIGNAL_MESSAGE] =
		g_signal_new(	"popup_message",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, popup_message),
						NULL, NULL,
						v3270_VOID__VOID_UINT_POINTER_POINTER_POINTER,
						G_TYPE_NONE, 4, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_FIELD] =
		g_signal_new(	"field_clicked",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270_BOOLEAN__VOID_BOOLEAN_UINT_POINTER,
						G_TYPE_BOOLEAN, 3, G_TYPE_BOOLEAN, G_TYPE_UINT, G_TYPE_POINTER);


	v3270_widget_signal[SIGNAL_PRINT_SETUP] =
		g_signal_new(	"print-begin",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_POINTER,
						G_TYPE_NONE, 1, G_TYPE_POINTER, 0);

	v3270_widget_signal[SIGNAL_PRINT_APPLY] =
		g_signal_new(	"print-apply",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_POINTER,
						G_TYPE_NONE, 1, G_TYPE_POINTER, 0);

	v3270_widget_signal[SIGNAL_PRINT_DONE] =
		g_signal_new(	"print-done",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270_VOID__VOID_POINTER_UINT,
						G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_UINT, 0);

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

	widget->host = lib3270_session_new("");
	lib3270_set_user_data(widget->host,widget);

	// Install callbacks
	v3270_install_callbacks(widget);

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
	widget->font.family = g_strdup(v3270_default_font);
	v3270_set_color_table(widget->color,v3270_default_colors);

}

LIB3270_EXPORT GtkWidget * v3270_new(void)
{
	return g_object_new(GTK_TYPE_V3270, NULL);
}

void v3270_clear_clipboard(v3270 *terminal)
{
    terminal->selection.text = lib3270_free(terminal->selection.text);
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

		// Release session
		lib3270_session_free(terminal->host);
		terminal->host = NULL;
	}

	if(terminal->accessible)
	{
		gtk_accessible_set_widget(terminal->accessible, NULL);
		g_object_unref(terminal->accessible);
		terminal->accessible = NULL;
	}

	if(terminal->font.family)
	{
		g_free(terminal->font.family);
		terminal->font.family = 0;
	}

	if(terminal->font.scaled)
	{
		cairo_scaled_font_destroy(terminal->font.scaled);
		terminal->font.scaled = NULL;
	}

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

    v3270_clear_clipboard(terminal);

	if(terminal->session_name)
	{
		g_free(terminal->session_name);
		terminal->session_name = NULL;
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

static void v3270_realize(GtkWidget	* widget)
{
#if GTK_CHECK_VERSION(2,18,0)
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

	}
#else
	{
		if(GTK_WIDGET_NO_WINDOW (widget))
		{
			GTK_WIDGET_CLASS(v3270_parent_class)->realize (widget);
		}
		else
		{
			GdkWindowAttr attributes;
			gint attributes_mask;

			GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

			memset(&attributes,0,sizeof(attributes));

			attributes.window_type = GDK_WINDOW_CHILD;
			attributes.x = widget->allocation.x;
			attributes.y = widget->allocation.y;
			attributes.width = widget->allocation.width;
			attributes.height = widget->allocation.height;
			attributes.wclass = GDK_INPUT_OUTPUT;
			attributes.visual = gtk_widget_get_visual (widget);
			attributes.colormap = gtk_widget_get_colormap (widget);
			attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

			attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

			widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),&attributes, attributes_mask);
			gdk_window_set_user_data(widget->window, widget);

			widget->style = gtk_style_attach (widget->style, widget->window);
			gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
		}

		gtk_im_context_set_client_window(GTK_V3270(widget)->input_method,widget->window);
	}

#endif // GTK(2,18,0)

#if !GTK_CHECK_VERSION(3,0,0)
	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
#endif // !GTK3

	v3270_reload(widget);

	v3270_send_configure(GTK_V3270(widget));

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

#if GTK_CHECK_VERSION(2,18,0)
	gtk_widget_set_allocation(widget, allocation);
#else
	widget->allocation = *allocation;
#endif // GTK(2,18)

#if !GTK_CHECK_VERSION(3,0,0)
	{
		v3270 *terminal = GTK_V3270(widget);

		terminal->width  = allocation->width;
		terminal->height = allocation->height;
	}
#endif

	if(gtk_widget_get_realized(widget))
	{
#if GTK_CHECK_VERSION(2,18,0)
		if(gtk_widget_get_has_window(widget))
			gdk_window_move_resize(gtk_widget_get_window (widget),allocation->x, allocation->y,allocation->width, allocation->height);
#else
		if(widget->window)
			gdk_window_move_resize(widget->window,allocation->x, allocation->y,allocation->width, allocation->height);
#endif // GTK(2,18,0)

		v3270_reload(widget);
		v3270_send_configure(GTK_V3270(widget));
	}
}

#if ! GTK_CHECK_VERSION(2,18,0)
G_GNUC_INTERNAL void gtk_widget_get_allocation(GtkWidget *widget, GtkAllocation *allocation)
{
	*allocation = widget->allocation;
}
#endif // !GTK(2,18)


static void v3270_send_configure(v3270 * terminal)
{
	GtkAllocation allocation;
	GtkWidget *widget;
	GdkEvent *event = gdk_event_new(GDK_CONFIGURE);

	widget = GTK_WIDGET(terminal);

	gtk_widget_get_allocation(widget, &allocation);

	event->configure.window = g_object_ref(gtk_widget_get_window(widget));
	event->configure.send_event = TRUE;
	event->configure.x = allocation.x;
	event->configure.y = allocation.y;
	event->configure.width = allocation.width;
	event->configure.height = allocation.height;

#if( !GTK_CHECK_VERSION(3,0,0))
	terminal->width  = allocation.width;
	terminal->height = allocation.height;
#endif

	gtk_widget_event(widget, event);
	gdk_event_free(event);
}

LIB3270_EXPORT void v3270_disconnect(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	v3270_disable_updates(widget);
	debug("%s",__FUNCTION__);
	lib3270_unselect(GTK_V3270(widget)->host);
	debug("%s",__FUNCTION__);
	lib3270_disconnect(GTK_V3270(widget)->host);
	debug("%s",__FUNCTION__);
	v3270_enable_updates(widget);
	debug("%s",__FUNCTION__);
}

LIB3270_EXPORT H3270 * v3270_get_session(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	return GTK_V3270(widget)->host;
}

LIB3270_EXPORT int v3270_reconnect(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

	return lib3270_reconnect(GTK_V3270(widget)->host,0);
}

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

static void v3270_activate(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

	terminal->activity.timestamp = time(0);

	if(lib3270_connected(terminal->host))
		lib3270_enter(terminal->host);
	else if(lib3270_get_hostname(terminal->host))
		v3270_reconnect(widget);
	else
		g_warning("Terminal widget %p activated without connection or valid hostname",terminal);
}

const GtkWidgetClass * v3270_get_parent_class(void)
{
	return GTK_WIDGET_CLASS(v3270_parent_class);
}

LIB3270_EXPORT GtkIMContext * v3270_get_im_context(GtkWidget *widget)
{
	return GTK_V3270(widget)->input_method;
}

/**
 * v3270_set_url:
 *
 * @widget:	V3270 widget.
 * @uri:	a valid tn3270 URL.
 *
 * Set the default URL for the tn3270e host.
 *
 * Since: 5.0
 **/
LIB3270_EXPORT void v3270_set_url(GtkWidget *widget, const gchar *uri)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	lib3270_set_url(GTK_V3270(widget)->host,uri);
}

LIB3270_EXPORT const gchar * v3270_get_hostname(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),"");
	return lib3270_get_hostname(GTK_V3270(widget)->host);
}

LIB3270_EXPORT const gchar	* v3270_get_luname(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),"");
	return lib3270_get_luname(GTK_V3270(widget)->host);
}

LIB3270_EXPORT const gchar	* v3270_get_session_name(GtkWidget *widget)
{
#ifdef DEBUG
	v3270 * terminal = GTK_V3270(widget);
	debug("Session name: [%s] Application name: [%s]",terminal->session_name, g_get_application_name());
#endif // DEBUG

	if(!(GTK_IS_V3270(widget) && GTK_V3270(widget)->session_name))
		return g_get_application_name();

	return GTK_V3270(widget)->session_name;
}

LIB3270_EXPORT void v3270_set_session_name(GtkWidget *widget, const gchar *name)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	g_return_if_fail(name != NULL);

	if(GTK_V3270(widget)->session_name)
		g_free(GTK_V3270(widget)->session_name);

	GTK_V3270(widget)->session_name = g_strdup(name);
}

LIB3270_EXPORT int v3270_set_host_type(GtkWidget *widget, LIB3270_HOST_TYPE type)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);
	return lib3270_set_host_type(GTK_V3270(widget)->host, type);
}

LIB3270_EXPORT int v3270_set_host_type_by_name(GtkWidget *widget, const char *name)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);
	return lib3270_set_host_type_by_name(GTK_V3270(widget)->host,name);
}

LIB3270_EXPORT gboolean v3270_is_connected(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return lib3270_connected(GTK_V3270(widget)->host) ? TRUE : FALSE;
}

LIB3270_EXPORT int v3270_set_host_charset(GtkWidget *widget, const gchar *name)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return lib3270_set_host_charset(GTK_V3270(widget)->host,name);
}

LIB3270_EXPORT GtkWidget * v3270_get_default_widget(void)
{
	H3270 * hSession = lib3270_get_default_session_handle();

	if(!hSession)
	{
		g_warning("%s: No default session available",__FUNCTION__);
		return NULL;
	}

	GtkWidget *widget = lib3270_get_user_data(hSession);

	if(!(widget && GTK_IS_V3270(widget)))
	{
		g_warning("%s: Can't determine default widget",__FUNCTION__);
		return NULL;
	}

	return GTK_WIDGET(widget);
}

