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

 #include <terminal.h>
 #include <internals.h>
 #include <v3270/dialogs.h>

 #include <gtk/gtk.h>
 #include <libintl.h>

 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/log.h>
 #include <lib3270/popup.h>
 #include <errno.h>
 #include <v3270/settings.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

struct has_timer
{
	H3270			* hSession;
	unsigned char	  on;
};

static gboolean bg_has_timer(struct has_timer *data)
{
	GtkWidget *widget = GTK_WIDGET(lib3270_get_user_data(data->hSession));

	if(data->on)
		v3270_start_timer(widget);
	else
		v3270_stop_timer(widget);

	g_free(data);

	return FALSE;
}

static void set_timer(H3270 *session, unsigned char on)
{
	struct has_timer *data = g_malloc0(sizeof(struct has_timer));

	data->hSession = session;
	data->on = on;

	g_idle_add((GSourceFunc) bg_has_timer, data);

}

static void update_toggle(H3270 *session, LIB3270_TOGGLE_ID id, unsigned char value, G_GNUC_UNUSED LIB3270_TOGGLE_TYPE reason, const char *name)
{
	v3270_update_toggle((GtkWidget *) lib3270_get_user_data(session), id, value, name);
}

static gboolean bg_update_message(H3270 *session)
{
	v3270 *terminal = (v3270 *) lib3270_get_user_data(session);

	v3270_signal_emit(
		terminal,
		V3270_SIGNAL_MESSAGE_CHANGED,
		(gint) lib3270_get_program_message(session)
	);

 	//trace("-----B %s %p",__FUNCTION__, lib3270_get_user_data(session));
 	return FALSE;
}

static void update_message(H3270 *session, G_GNUC_UNUSED LIB3270_MESSAGE id)
{
	g_idle_add((GSourceFunc) bg_update_message, session);
}

static void update_luname(H3270 *session, const char G_GNUC_UNUSED(*name))
{
	g_idle_add((GSourceFunc) v3270_update_associated_lu, lib3270_get_user_data(session));
}

static gboolean	v3270_update_url(v3270 *terminal)
{
	GtkWidget * widget = GTK_WIDGET(terminal);
	debug("url=%s",v3270_get_url(widget));

	v3270_emit_save_settings(widget,"url");
	v3270_signal_emit(widget, V3270_SIGNAL_SESSION_CHANGED);
	return FALSE;
}

static void update_url(H3270 *session, const char G_GNUC_UNUSED(*name))
{
	g_idle_add((GSourceFunc) v3270_update_url, lib3270_get_user_data(session));
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

	if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
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
		v3270_signal_emit(GTK_WIDGET(widget), V3270_SIGNAL_CONNECTED, lib3270_get_url(session));
	}
	else
	{
		widget->cursor.show &= ~2;
		v3270_signal_emit(GTK_WIDGET(widget), V3270_SIGNAL_DISCONNECTED);
	}

	debug("%s(%p)",__FUNCTION__,GTK_V3270_GET_CLASS(widget)->properties.online);
	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.online);

	widget->activity.timestamp = time(0);

	gtk_widget_queue_draw(GTK_WIDGET(widget));
}

static void update_screen_size(H3270 *session, G_GNUC_UNUSED unsigned short rows, G_GNUC_UNUSED unsigned short cols)
{
	debug("%s",__FUNCTION__);
	GtkWidget * widget = GTK_WIDGET(lib3270_get_user_data(session));

	v3270_reconfigure(GTK_V3270(widget));
	gtk_widget_queue_draw(widget);

}

static void update_model(H3270 *session, const char *name, int model, G_GNUC_UNUSED int rows, G_GNUC_UNUSED int cols)
{
	GtkWidget * widget = GTK_WIDGET(lib3270_get_user_data(session));

//	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.settings[V3270_SETTING_MODEL_NUMBER]);
	v3270_signal_emit(widget,V3270_SIGNAL_MODEL_CHANGED, (guint) model, name);
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

	v3270_signal_emit(GTK_WIDGET(widget),V3270_SIGNAL_CHANGED, (guint) offset, (guint) len);
}

static void set_selection(H3270 *session, unsigned char status)
{
	GtkWidget * widget = GTK_WIDGET(lib3270_get_user_data(session));

	debug("%s(%p)",__FUNCTION__,GTK_V3270_GET_CLASS(widget)->properties.selection);
	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.selection);

	v3270_signal_emit(widget,V3270_SIGNAL_SELECTING, status ? TRUE : FALSE);

}

static void update_selection(H3270 *session, G_GNUC_UNUSED int start, G_GNUC_UNUSED int end)
{
	// Selected region changed
	GtkWidget		* widget	= GTK_WIDGET(lib3270_get_user_data(session));
	GtkAccessible	* atk_obj	= GTK_V3270(widget)->accessible;

	if(atk_obj)
		g_signal_emit_by_name(atk_obj,"text-selection-changed");

}

/*
static void message(H3270 *session, LIB3270_NOTIFY type , const char *title, const char *message, const char *text)
{
	LIB3270_POPUP popup = {
		.type = type,
		.title = title,
		.summary = message,
		.body = text
	};

	v3270_popup_dialog_show(GTK_WIDGET(lib3270_get_user_data(session)),&popup,0);

}
*/

static int print(H3270 *session, LIB3270_CONTENT_OPTION mode)
{
	return v3270_print_dialog(GTK_WIDGET(lib3270_get_user_data(session)), mode, NULL);
}

static int save(H3270 *session, LIB3270_CONTENT_OPTION mode, const char *filename)
{
	GtkWidget * widget = GTK_WIDGET(lib3270_get_user_data(session));

	if(!GTK_IS_V3270(widget))
		return errno = EINVAL;

	GtkWidget *dialog = v3270_save_dialog_new(widget,mode,filename);
	gtk_widget_show_all(dialog);
	v3270_save_dialog_run(dialog);
	gtk_widget_destroy(dialog);

	return 0;
}

static int load(H3270 *session, const char *filename)
{
	GtkWidget * widget = GTK_WIDGET(lib3270_get_user_data(session));

	if(!GTK_IS_V3270(widget))
		return errno = EINVAL;

	GtkWidget *dialog = v3270_load_dialog_new(widget,filename);
	gtk_widget_show_all(dialog);
	v3270_load_dialog_run(dialog);
	gtk_widget_destroy(dialog);

	return 0;
}

/*
static void popup_handler(H3270 *session, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list args)
{
	LIB3270_POPUP popup = {
		.type = type,
		.title = title,
		.summary = msg
	};

	g_autofree gchar * body = NULL;

	if(fmt) {
		body = g_strdup_vprintf(fmt,args);
		popup.body = body;
	}

	v3270_popup_dialog_show(GTK_WIDGET(lib3270_get_user_data(session)),&popup,0);

 }
 */

 static gboolean bg_update_ssl(H3270 *session)
 {
 	v3270 *terminal = GTK_V3270(lib3270_get_user_data(session));

 	if(terminal->surface)
	{
		// Redraw SSL area.
		GdkRectangle	* r;
		cairo_t			* cr = v3270_oia_set_update_region(terminal,&r,V3270_OIA_SSL);

		v3270_draw_ssl_status(terminal,cr,r);
		v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
		cairo_destroy(cr);

	}

 	if(v3270_blink_ssl(terminal))
		v3270_start_blinking(GTK_WIDGET(lib3270_get_user_data(session)));

	return FALSE;
 }

 static void update_ssl(H3270 *session, G_GNUC_UNUSED LIB3270_SSL_STATE state)
 {
 	debug("----------------------> %d", (int) state);

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

 static int popup(H3270 *hSession, const LIB3270_POPUP *popup, unsigned char wait) {

	GtkResponseType response = v3270_popup_dialog_show(
									GTK_WIDGET(lib3270_get_user_data(hSession)),
									popup,
									wait != 0);

	if(response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY)
		return 0;

	return errno = ECANCELED;

 }

 static int action(H3270 *hSession, const char *name) {

	guint response = ENOENT;

	v3270_signal_emit(
		GTK_WIDGET(lib3270_get_user_data(hSession)),
		V3270_SIGNAL_FIRE_ACTION,
		name,
		&response
	);

	return response;
 }


 void v3270_install_callbacks(v3270 *widget)
 {
	struct lib3270_session_callbacks *cbk;

	cbk = lib3270_get_session_callbacks(widget->host,G_STRINGIFY(LIB3270_REVISION),sizeof(struct lib3270_session_callbacks));
	if(!cbk)
	{
		if(g_ascii_strcasecmp(G_STRINGIFY(LIB3270_REVISION),lib3270_get_revision()))
		{
			g_error(
				_("Invalid callback table, the release %s of lib%s can't be used (expecting revision %s)"),
				lib3270_get_revision(),
				G_STRINGIFY(LIB3270_NAME),
				G_STRINGIFY(LIB3270_REVISION)
			);
		}
		else
		{
			g_error(
				_("Unexpected callback table, the release %s of lib%s is invalid"),
				lib3270_get_revision(),
				G_STRINGIFY(LIB3270_NAME)
			);
		}

		return;
	}

	cbk->update				= v3270_update_char;
	cbk->changed			= changed;
	cbk->set_timer 			= set_timer;

	cbk->set_selection		= set_selection;
	cbk->update_selection	= update_selection;

	cbk->update_luname		= update_luname;
	cbk->update_url			= update_url;

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
	cbk->update_ssl			= update_ssl;
	cbk->print				= print;
	cbk->save				= save;
	cbk->load				= load;
	cbk->popup				= popup;
	cbk->action				= action;

}

