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

 void v3270_get_property(GObject *object,guint prop_id, GValue *value, GParamSpec *pspec)
 {
	v3270		* window	= GTK_V3270(object);
 	v3270Class	* klass		= GTK_V3270_GET_CLASS(object);

 	debug("%s(%u,%s)",__FUNCTION__,prop_id,g_param_spec_get_name(pspec));

 	if(prop_id >= klass->properties.type.str)
	{
		const LIB3270_STRING_PROPERTY * prop = (lib3270_get_string_properties_list()+(prop_id - klass->properties.type.str));
		debug("%s.%s.%s",__FUNCTION__,"string",prop->name);

		if(prop->get)
			g_value_set_string(value,prop->get(window->host));

	}
	else if(prop_id >= klass->properties.type.uint)
	{
		const LIB3270_UINT_PROPERTY * prop = (lib3270_get_unsigned_properties_list()+(prop_id - klass->properties.type.uint));
		debug("%s.%s.%s",__FUNCTION__,"unsigned",prop->name);

		if(prop->get)
			g_value_set_uint(value,prop->get(window->host));

	}
	else if(prop_id >= klass->properties.type.integer)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_int_properties_list()+(prop_id - klass->properties.type.integer));
		debug("%s.%s.%s",__FUNCTION__,"integer",prop->name);

		if(prop->get)
			g_value_set_int(value,prop->get(window->host));

	}
	else if(prop_id >= klass->properties.type.boolean)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_boolean_properties_list()+(prop_id - klass->properties.type.boolean));
		debug("%s.%s.%s",__FUNCTION__,"boolean",prop->name);

		if(prop->get)
			g_value_set_boolean(value,prop->get(window->host) != 0 ? TRUE : FALSE);

	}
 	else if(prop_id >= klass->properties.type.toggle)
	{
		debug("%s.%s.%s",__FUNCTION__,"toggle",lib3270_get_toggle_name(prop_id - klass->properties.type.toggle));
		g_value_set_boolean(value,lib3270_get_toggle(window->host,prop_id - klass->properties.type.toggle) ? TRUE : FALSE );

	}
	else
	{
		// Check for internal properties.
		switch(prop_id) {
		case V3270_PROPERTY_FONT_FAMILY:	// Font-family
			g_value_set_string(value,v3270_get_font_family(GTK_WIDGET(object)));
			break;

		case V3270_PROPERTY_CLIPBOARD:	// Clipboard
			g_value_take_string(value,gdk_atom_name(window->selection.target));
			break;

		case V3270_PROPERTY_SESSION_NAME:
			g_value_set_string(value,v3270_get_session_name(GTK_WIDGET(object)));
			break;

		case V3270_PROPERTY_AUTO_DISCONNECT:
			g_value_set_uint(value,window->activity.disconnect);
			break;

		case V3270_PROPERTY_REMAP_FILE:		// Remap file
			g_value_set_string(value,v3270_get_remap_filename(GTK_WIDGET(object)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);

		}
	}

 }

LIB3270_EXPORT const gchar * v3270_get_url(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return lib3270_get_url(GTK_V3270(widget)->host);
}

LIB3270_EXPORT const gchar * v3270_get_luname(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),"");
	return lib3270_get_luname(GTK_V3270(widget)->host);
}

LIB3270_EXPORT const gchar * v3270_get_session_name(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	v3270 * terminal = GTK_V3270(widget);

	if(terminal->session_name)
		return terminal->session_name;

	char id[] = { lib3270_get_session_id(terminal->host), 0 };
	if(id[0])
		return (terminal->session_name = g_strconcat(G_STRINGIFY(PRODUCT_NAME),":",id,NULL));

	return G_STRINGIFY(PRODUCT_NAME);

}

LIB3270_EXPORT H3270 * v3270_get_session(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	return GTK_V3270(widget)->host;
}

LIB3270_EXPORT gboolean v3270_is_connected(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return lib3270_is_connected(GTK_V3270(widget)->host) ? TRUE : FALSE;
}

LIB3270_EXPORT guint v3270_get_auto_disconnect(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),0);
 	return GTK_V3270(widget)->activity.disconnect;
}



