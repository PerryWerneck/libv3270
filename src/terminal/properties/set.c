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

 #include "private.h"
 #include <v3270/trace.h>
 #include <v3270/settings.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
 {
	v3270 		* window	= GTK_V3270(object);
 	v3270Class	* klass		= GTK_V3270_GET_CLASS(object);

 	debug("%s(%u,%s)",__FUNCTION__,prop_id,g_param_spec_get_name(pspec));

 	if(prop_id >= klass->properties.type.str)
	{
		const LIB3270_STRING_PROPERTY * prop = (lib3270_get_string_properties_list()+(prop_id - klass->properties.type.str));
		debug("%s.%s.%s=%s",__FUNCTION__,"string",prop->name,g_value_get_string(value));

		if(prop->set)
			prop->set(window->host,g_value_get_string(value));

	}
	else if(prop_id >= klass->properties.type.uint)
	{
		const LIB3270_UINT_PROPERTY * prop = (lib3270_get_unsigned_properties_list()+(prop_id - klass->properties.type.uint));
		debug("%s.%s.%s",__FUNCTION__,"unsigned",prop->name);

		if(prop->set)
			prop->set(window->host,g_value_get_uint(value));

	}
	else if(prop_id >= klass->properties.type.integer)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_int_properties_list()+(prop_id - klass->properties.type.integer));
		debug("%s.%s.%s",__FUNCTION__,"integer",prop->name);

		if(prop->set)
			prop->set(window->host,g_value_get_int(value));

	}
	else if(prop_id >= klass->properties.type.boolean)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_boolean_properties_list()+(prop_id - klass->properties.type.boolean));
		debug("%s.%s.%s",__FUNCTION__,"boolean",prop->name);

		if(prop->set)
			prop->set(window->host,g_value_get_boolean(value) ? 1 : 0);

	}
 	else if(prop_id >= klass->properties.type.toggle)
	{
		debug("%s.%s=%s",__FUNCTION__,"toggle",g_value_get_boolean(value) ? "ON" : "OFF");
		lib3270_set_toggle(window->host,prop_id - klass->properties.type.toggle, (int) g_value_get_boolean(value));
	}
	else
	{
		// Check for internal properties.
		switch(prop_id) {
		case V3270_PROPERTY_FONT_FAMILY:	// Font-family
			v3270_set_font_family(GTK_WIDGET(object), g_value_get_string(value));
			break;

		case V3270_PROPERTY_CLIPBOARD:	// Clipboard
			{
				const gchar * name = g_value_get_string(value);
				if(!*name) {
					g_message("Setting default clipboard");
					window->selection.target = GDK_SELECTION_CLIPBOARD;
				}
				else
			{
					GdkAtom clipboard = gdk_atom_intern(name,TRUE);
					if(clipboard == GDK_NONE)
					{
						g_warning("\"%s\" is not a valid clipboard name",name);
					}
					else
					{
						window->selection.target = clipboard;
					}
				}
			}
			break;

		case V3270_PROPERTY_SESSION_NAME:	// Session Name
			v3270_set_session_name(GTK_WIDGET(object), g_value_get_string(value));
			break;

		case V3270_PROPERTY_AUTO_DISCONNECT:
			v3270_set_auto_disconnect(GTK_WIDGET(object), g_value_get_uint(value));
			break;

		case V3270_PROPERTY_REMAP_FILE:		// Remap file
			v3270_set_remap_filename(GTK_WIDGET(object), g_value_get_string(value));
			break;

		case V3270_PROPERTY_DYNAMIC_SPACING:
			v3270_set_dynamic_font_spacing(GTK_WIDGET(object), g_value_get_boolean(value));
			break;

		case V3270_PROPERTY_LU_NAMES:
			v3270_set_lunames(GTK_WIDGET(object),g_value_get_string(value));
			break;

		case V3270_PROPERTY_TRACE:
			v3270_set_trace(GTK_WIDGET(object), g_value_get_boolean(value));
			break;

		case V3270_PROPERTY_TERMINAL_COLORS:
			v3270_set_colors(GTK_WIDGET(object),g_value_get_string(value));
			break;

		case V3270_PROPERTY_SELECTION_OPTIONS:
			GTK_V3270(object)->selection.options = (V3270SelectionOption) g_value_get_uint(value);
			break;

		case V3270_PROPERTY_OPEN_URL:
			GTK_V3270(object)->open_url = (g_value_get_boolean(value) ? 1 : 0);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);

		}

	}


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

LIB3270_EXPORT void v3270_set_session_name(GtkWidget *widget, const gchar *name)
{
	debug("%s(%s)",__FUNCTION__,name);

	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	if(!(name && *name))
		name = G_STRINGIFY(PRODUCT_NAME);

	if(terminal->session.name) {

		// If it's the same name ignore it.
		if(!strcmp(terminal->session.name,name))
			return;

		g_free(terminal->session.name);
		terminal->session.name = NULL;

	}

	g_autofree gchar * new_name = g_strdup(name);

	// Check for session id
	gchar session_id = 0;
	{
		gchar *ptr = strchr(new_name,':');
		if(ptr) {
			*(ptr++) = 0;
			session_id = *ptr;
			lib3270_set_session_id(terminal->host,session_id);
		}
	}

	if(!session_id)
		session_id = lib3270_get_session_id(terminal->host);

	debug("%s new_name=%s",__FUNCTION__,new_name);

	if(session_id) {
		terminal->session.name = g_strdup_printf("%s:%c",new_name,session_id);
	} else {
		terminal->session.name = g_strdup(new_name);
	}

	g_message("Session name changes to \"%s\"",terminal->session.name);

	v3270_signal_emit(GTK_WIDGET(widget), V3270_SIGNAL_SESSION_CHANGED);
	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.session_name);

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

LIB3270_EXPORT int v3270_set_host_charset(GtkWidget *widget, const gchar *name)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return lib3270_set_host_charset(GTK_V3270(widget)->host,name);
}

void v3270_set_cursor(GtkWidget *widget, LIB3270_POINTER id)
{
	gdk_window_set_cursor(
		gtk_widget_get_window(widget),
		GTK_V3270_GET_CLASS(widget)->cursors[id % LIB3270_POINTER_COUNT]
	);
}

LIB3270_EXPORT void v3270_set_auto_disconnect(GtkWidget *widget, guint minutes)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	if(terminal->activity.disconnect != minutes)
	{
		terminal->activity.disconnect = minutes;
 		v3270_emit_save_settings(widget,"auto_disconnect");
	}

}

LIB3270_EXPORT void	v3270_set_dynamic_font_spacing(GtkWidget *widget, gboolean state)
{

	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	if(terminal->font.spacing.dynamic != state)
	{
		terminal->font.spacing.dynamic = state;
		v3270_reconfigure(terminal);
		gtk_widget_queue_draw(widget);
		v3270_emit_save_settings(widget,"dynamic_font_spacing");
	}

}

LIB3270_EXPORT void v3270_set_lunames(GtkWidget *widget, const gchar *lunames)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	lib3270_set_lunames(GTK_V3270(widget)->host,(lunames && *lunames ? lunames : NULL));
	v3270_emit_save_settings(widget,"lu_names");
}

