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

 #ifdef _WIN32
	#include <winsock2.h>
 #endif // _WIN32

 #include "private.h"

 #include <gtk/gtk.h>
 #include <libintl.h>
 #include <glib/gi18n.h>

 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/log.h>
 #include <errno.h>

/**
 * SECTION:	v3270
 * @title:	Virtual 3270 widget
 * @short_description:	The virtual 3270 terminal widget.
 *
 * Manages lib3270 callbacks.
 *
 */

/*--[ Widget definition ]----------------------------------------------------------------------------*/

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void set_timer(H3270 *session, unsigned char on)
{
	GtkWidget *widget = GTK_WIDGET(lib3270_get_user_data(session));

	if(on)
		v3270_start_timer(widget);
	else
		v3270_stop_timer(widget);

}

static void update_toggle(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, G_GNUC_UNUSED LIB3270_TOGGLE_TYPE reason, const char *name)
{
	GtkWidget *widget = GTK_WIDGET(lib3270_get_user_data(session));

	switch(ix)
	{
	case LIB3270_TOGGLE_CURSOR_POS:
	case LIB3270_TOGGLE_MONOCASE:
	case LIB3270_TOGGLE_LINE_WRAP:
	case LIB3270_TOGGLE_CROSSHAIR:
	case LIB3270_TOGGLE_BLANK_FILL:
	case LIB3270_TOGGLE_MARGINED_PASTE:
	case LIB3270_TOGGLE_SHOW_TIMING:
	case LIB3270_TOGGLE_RECTANGLE_SELECT:
	case LIB3270_TOGGLE_UNDERLINE:
	case LIB3270_TOGGLE_VIEW_FIELD:
	case LIB3270_TOGGLE_ALTSCREEN:
		v3270_reload(widget);
		gtk_widget_queue_draw(widget);
		break;

	case LIB3270_TOGGLE_CURSOR_BLINK:
		GTK_V3270(widget)->cursor.show |= 1;
		break;

	case LIB3270_TOGGLE_INSERT:
		v3270_draw_ins_status(GTK_V3270(widget));
		v3270_cursor_draw(GTK_V3270(widget));
		break;

	case LIB3270_TOGGLE_BOLD:
		v3270_reload(widget);
		gtk_widget_queue_draw(widget);
		break;

	case LIB3270_TOGGLE_FULL_SCREEN:
		if(value)
			gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		else
			gtk_window_unfullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));

		break;

	case LIB3270_TOGGLE_DS_TRACE:
	case LIB3270_TOGGLE_SSL_TRACE:
	case LIB3270_TOGGLE_SCREEN_TRACE:
	case LIB3270_TOGGLE_EVENT_TRACE:
	case LIB3270_TOGGLE_RECONNECT:
	case LIB3270_TOGGLE_SMART_PASTE:
	case LIB3270_TOGGLE_KEEP_SELECTED:
	case LIB3270_TOGGLE_CONNECT_ON_STARTUP:
	case LIB3270_TOGGLE_KP_ALTERNATIVE:
	case LIB3270_TOGGLE_NETWORK_TRACE:
	case LIB3270_TOGGLE_BEEP:
	case LIB3270_TOGGLE_KEEP_ALIVE:
		break;

	case LIB3270_TOGGLE_COUNT:
		break;

	}

	g_object_notify_by_pspec(G_OBJECT(widget), v3270_properties.toggle[ix]);
	g_signal_emit(widget, v3270_widget_signal[SIGNAL_TOGGLE_CHANGED], 0, (guint) ix, (gboolean) (value != 0), (gchar *) name);

}

static gboolean bg_update_message(H3270 *session)
{
	void *widget = lib3270_get_user_data(session);
 	trace("-----A %s %p",__FUNCTION__, lib3270_get_user_data(session));

	g_signal_emit(
		GTK_WIDGET(widget),
		v3270_widget_signal[SIGNAL_MESSAGE_CHANGED],
		0,
		(gint) lib3270_get_program_message(session)
	);

 	trace("-----B %s %p",__FUNCTION__, lib3270_get_user_data(session));

 	return FALSE;
}

static void update_message(H3270 *session, G_GNUC_UNUSED LIB3270_MESSAGE id)
{
	g_idle_add((GSourceFunc) bg_update_message, session);
}

static void update_luname(H3270 *session, const char *name)
{
	v3270_update_luname(GTK_WIDGET(lib3270_get_user_data(session)),name);
}

struct select_cursor_data
{
	H3270			* hSession;
	LIB3270_POINTER   id;
};

static gboolean bg_select_cursor(struct select_cursor_data *data)
{
	GtkWidget *widget = GTK_WIDGET(lib3270_get_user_data(data->hSession));

#if GTK_CHECK_VERSION(2,20,0)
	if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
#else
	if(GTK_WIDGET_REALIZED(widget) && widget->window)
#endif // GTK(2,20)
	{
		GTK_V3270(widget)->pointer_id = data->id;
		v3270_update_mouse_pointer(widget);
	}

	return FALSE;
}

static void select_cursor(H3270 *session, LIB3270_POINTER id)
{
	struct select_cursor_data *data = g_new0(struct select_cursor_data,1);
	data->hSession = session;
	data->id = id;
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,(GSourceFunc) bg_select_cursor, data, g_free);
}

static void ctlr_done(H3270 *session)
{
	GtkWidget *widget = GTK_WIDGET(lib3270_get_user_data(session));

#if GTK_CHECK_VERSION(2,20,0)
	if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
#else
	if(GTK_WIDGET_REALIZED(widget) && widget->window)
#endif // GTK(2,20)
	{
		v3270_update_mouse_pointer(widget);
	}

}

static void update_connect(H3270 *session, unsigned char connected)
{
	v3270 *widget = GTK_V3270(lib3270_get_user_data(session));

	if(connected)
	{
		widget->cursor.show |= 2;
		g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_CONNECTED], 0, lib3270_get_host(session));
	}
	else
	{
		widget->cursor.show &= ~2;
		g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_DISCONNECTED], 0);
	}

	if(v3270_properties.online)
		g_object_notify_by_pspec(G_OBJECT(widget), v3270_properties.online);

	widget->activity.timestamp = time(0);

	gtk_widget_queue_draw(GTK_WIDGET(widget));
}

static void update_screen_size(H3270 *session, G_GNUC_UNUSED unsigned short rows, G_GNUC_UNUSED unsigned short cols)
{
	v3270_reload(GTK_WIDGET(lib3270_get_user_data(session)));
	gtk_widget_queue_draw(GTK_WIDGET(lib3270_get_user_data(session)));
}

static void update_model(H3270 *session, const char *name, int model, G_GNUC_UNUSED int rows, G_GNUC_UNUSED int cols)
{
	if(v3270_properties.model)
		g_object_notify_by_pspec(G_OBJECT(lib3270_get_user_data(session)), v3270_properties.model);

	g_signal_emit(GTK_WIDGET(lib3270_get_user_data(session)),v3270_widget_signal[SIGNAL_MODEL_CHANGED], 0, (guint) model, name);
}

static void changed(H3270 *session, int offset, int len)
{
	GtkWidget 		* widget	= lib3270_get_user_data(session);
	GtkAccessible	* obj		= GTK_V3270(widget)->accessible;

	if(obj)
	{
		// Get new text, notify atk
		gsize	  bytes_written	= 0;
		char	* text 			= lib3270_get_string_at_address(session,offset,len,'\n');

		if(text)
		{
			GError	* error		= NULL;
			gchar	* utfchar	= g_convert_with_fallback(	text,
																-1,
																"UTF-8",
																lib3270_get_display_charset(session),
																" ",
																NULL,
																&bytes_written,
																&error );

			lib3270_free(text);

			if(error)
			{
				g_warning("%s failed: %s",__FUNCTION__,error->message);
				g_error_free(error);
			}

			if(utfchar)
			{
				g_signal_emit_by_name(obj, "text-insert", offset, bytes_written, utfchar);
				g_free(utfchar);
			}

		}
	}

#ifdef WIN32
	gtk_widget_queue_draw(widget);
#endif // WIN32

	g_signal_emit(GTK_WIDGET(widget),v3270_widget_signal[SIGNAL_CHANGED], 0, (guint) offset, (guint) len);
}

static void set_selection(H3270 *session, unsigned char status)
{
	GtkWidget * widget = GTK_WIDGET(lib3270_get_user_data(session));

	if(v3270_properties.selection)
		g_object_notify_by_pspec(G_OBJECT(widget), v3270_properties.selection);

	g_signal_emit(widget,v3270_widget_signal[SIGNAL_SELECTING], 0, status ? TRUE : FALSE);

}

static void update_selection(H3270 *session, G_GNUC_UNUSED int start, G_GNUC_UNUSED int end)
{
	// Selected region changed
	GtkWidget		* widget	= GTK_WIDGET(lib3270_get_user_data(session));
	GtkAccessible	* atk_obj	= GTK_V3270(widget)->accessible;

	if(atk_obj)
		g_signal_emit_by_name(atk_obj,"text-selection-changed");

}

static void message(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *text)
{
	g_signal_emit(	GTK_WIDGET(lib3270_get_user_data(session)), v3270_widget_signal[SIGNAL_MESSAGE], 0,
							(int) id,
							(gchar *) title,
							(gchar *) message,
							(gchar *) text );

}


static int print(H3270 *session, LIB3270_PRINT_MODE mode)
{
	GtkWidget	* widget = GTK_WIDGET(lib3270_get_user_data(session));
	GError		* error = NULL;

	v3270_print(widget, mode, &error);

	if(error)
	{
		GtkWidget *dialog =
			gtk_message_dialog_new_with_markup(
						GTK_WINDOW(gtk_widget_get_toplevel(widget)),
						GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
						_( "Operation has failed" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_("Can't print"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",error->message);

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		g_error_free(error);
		return -1;

	}

	return 0;
}

static void popup_handler(H3270 *session, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list args)
{
 	GtkWidget *terminal = (GtkWidget *) lib3270_get_user_data(session);

 	if(terminal && GTK_IS_V3270(terminal)) {

		if(fmt)
		{
			gchar *text = g_strdup_vprintf(fmt,args);
			v3270_popup_message(GTK_WIDGET(terminal),type,title,msg,text);
			g_free(text);
		}
		else
		{
			v3270_popup_message(GTK_WIDGET(terminal),type,title,msg,NULL);
		}

 	}

 }

 static gboolean bg_update_ssl(H3270 *session)
 {
 	trace("%s(%p,%p)",__FUNCTION__,session,lib3270_get_user_data(session));

 	v3270_blink_ssl(GTK_V3270(lib3270_get_user_data(session)));

	if(lib3270_get_secure(session) == LIB3270_SSL_NEGOTIATING)
		v3270_start_blinking(GTK_WIDGET(lib3270_get_user_data(session)));

	return FALSE;
 }

 static void update_ssl(H3270 *session, G_GNUC_UNUSED LIB3270_SSL_STATE state)
 {
	g_idle_add((GSourceFunc) bg_update_ssl, session);
 }

 struct update_oia_data
 {
 	H3270 *session;
	LIB3270_FLAG id;
	unsigned char on;
 };

 static gboolean bg_update_oia(struct update_oia_data *data)
 {
  	v3270_update_oia(GTK_V3270(lib3270_get_user_data(data->session)), data->id, data->on);
 	return FALSE;
 }

 static void update_oia(H3270 *session, LIB3270_FLAG id, unsigned char on)
 {
	struct update_oia_data *data = g_new0(struct update_oia_data,1);
	data->session = session;
	data->id = id;
	data->on = on;
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,(GSourceFunc) bg_update_oia, data, g_free);
 }

 void v3270_install_callbacks(v3270 *widget)
 {
	struct lib3270_session_callbacks *cbk;

	lib3270_set_popup_handler(widget->host, popup_handler);

	cbk = lib3270_get_session_callbacks(widget->host,sizeof(struct lib3270_session_callbacks));
	if(!cbk)
	{
		g_error( _( "Invalid callback table, possible version mismatch in lib3270") );
		return;
	}

	cbk->update				= v3270_update_char;
	cbk->changed			= changed;
	cbk->set_timer 			= set_timer;

	cbk->set_selection		= set_selection;
	cbk->update_selection	= update_selection;

	cbk->update_luname		= update_luname;
	cbk->configure			= update_screen_size;
	cbk->update_status 		= update_message;
	cbk->update_cursor 		= v3270_update_cursor;
	cbk->update_toggle 		= update_toggle;
	cbk->update_oia			= update_oia;
	cbk->cursor				= select_cursor;
	cbk->update_connect		= update_connect;
	cbk->update_model		= update_model;
	cbk->changed			= changed;
	cbk->ctlr_done			= ctlr_done;
	cbk->message			= message;
	cbk->update_ssl			= update_ssl;
	cbk->print				= print;

}

