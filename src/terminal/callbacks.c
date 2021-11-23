/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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


struct update_message_data
{
	H3270 *hSession;
	LIB3270_MESSAGE message;
};

static gboolean bg_update_message(struct update_message_data *data)
{
	v3270 *terminal = (v3270 *) lib3270_get_user_data(data->hSession);

	v3270_signal_emit(
		terminal,
		V3270_SIGNAL_MESSAGE_CHANGED,
		(gint) data->message
	);

	g_free(data);
 	return FALSE;
}

static void update_message(H3270 *hSession, LIB3270_MESSAGE message)
{
	struct update_message_data *data = g_new0(struct update_message_data,1);
	data->hSession = hSession;
	data->message = message;

	g_idle_add((GSourceFunc) bg_update_message, data);
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

 struct _bg_reconnect
 {
	H3270 *hSession;
	int seconds;
 };

 static gboolean bg_reconnect(struct _bg_reconnect *cfg)
 {
	lib3270_reconnect(cfg->hSession,cfg->seconds);
	return G_SOURCE_REMOVE;
 }

 static int reconnect(H3270 *hSession,int seconds)
 {
 	struct _bg_reconnect *cfg = g_new0(struct _bg_reconnect, 1);

 	cfg->hSession = hSession;
 	cfg->seconds = seconds;

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,(GSourceFunc) bg_reconnect, cfg, g_free);

 	return 0;
 }

 struct _word_selected
 {
	H3270 *hSession;
	int offset;
	int len;
 };

 static gboolean bg_word_selected(struct _word_selected *cfg)
 {
 	v3270 *terminal = (v3270 *) lib3270_get_user_data(cfg->hSession);

 	debug("%s(open-url=%d)",__FUNCTION__,terminal->open_url);

	if(cfg->len > 3 && terminal->open_url) {

		lib3270_autoptr(char) text = lib3270_get_string_at_address(cfg->hSession,cfg->offset,cfg->len,0);

		if(text && (g_str_has_prefix(text,"http://") || g_str_has_prefix(text,"https://")) ) {

			debug("Emitting '%s'", text);

			guint response = 0;
			v3270_signal_emit(
				terminal,
				V3270_SIGNAL_OPEN_URL,
				text,
				&response
			);

			debug("Response was: %d", (int) response);

			if(response == 0) {
				// No one has changed the response, take default action.
				g_message("Opening '%s'",text);
				if(gtk_show_uri_on_window(
								GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(terminal))),
								text,
								GDK_CURRENT_TIME,
								NULL
						)) {
					v3270_unselect(GTK_WIDGET(terminal));
				}
			}

		}

	}

	return G_SOURCE_REMOVE;
 }

 static void word_selected(H3270 *hSession, int from, int to) {

 	struct _word_selected *cfg = g_new0(struct _word_selected, 1);

 	cfg->hSession = hSession;
 	cfg->offset = from;
 	cfg->len = (to-from)+1;

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,(GSourceFunc) bg_word_selected, cfg, g_free);

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
	cbk->reconnect			= reconnect;
	cbk->word_selected		= word_selected;

}

