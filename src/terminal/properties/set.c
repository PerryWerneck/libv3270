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
 * Este programa está nomeado como properties.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

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
		debug("%s.%s",__FUNCTION__,"toggle");
		lib3270_set_toggle(window->host,prop_id - klass->properties.type.toggle, (int) g_value_get_boolean (value));

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
	g_return_if_fail(GTK_IS_V3270(widget));
	g_return_if_fail(name != NULL);

	if(GTK_V3270(widget)->session.name) {

		if(!strcmp(GTK_V3270(widget)->session.name,name)) {
			// Same session name, keep it.
			return;
		}

		g_free(GTK_V3270(widget)->session.name);

	}

	GTK_V3270(widget)->session.name = g_strdup(name);

	g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[V3270_SIGNAL_SESSION_CHANGED], 0);
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
 		g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.auto_disconnect);
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
 		g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.dynamic_spacing);
	}

}

LIB3270_EXPORT void v3270_set_lunames(GtkWidget *widget, const gchar *lunames)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	lib3270_set_lunames(GTK_V3270(widget)->host,(lunames && *lunames ? lunames : NULL));
	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.lu_names);
}

