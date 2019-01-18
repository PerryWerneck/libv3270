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

#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
#endif // WIN32

 #include <gtk/gtk.h>
 #include <limits.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/properties.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <v3270.h>
 #include "private.h"

 #define PROP_BEGIN 2

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
 {
	v3270  *window = GTK_V3270(object);

 	debug("%s(%u,%s)",__FUNCTION__,prop_id,g_param_spec_get_name(pspec));

 	if(prop_id >= v3270_properties.type.str)
	{
		const LIB3270_STRING_PROPERTY * prop = (lib3270_get_string_properties_list()+(prop_id - v3270_properties.type.str));
		debug("%s.%s.%s=%s",__FUNCTION__,"string",prop->name,g_value_get_string(value));

		if(prop->set)
			prop->set(window->host,g_value_get_string(value));

	}
	else if(prop_id >= v3270_properties.type.integer)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_int_properties_list()+(prop_id - v3270_properties.type.integer));
		debug("%s.%s.%s",__FUNCTION__,"integer",prop->name);

		if(prop->set)
			prop->set(window->host,g_value_get_int(value));

	}
	else if(prop_id >= v3270_properties.type.boolean)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_boolean_properties_list()+(prop_id - v3270_properties.type.boolean));
		debug("%s.%s.%s",__FUNCTION__,"boolean",prop->name);

		if(prop->set)
			prop->set(window->host,g_value_get_boolean(value) ? 1 : 0);

	}
 	else if(prop_id >= v3270_properties.type.toggle)
	{
		debug("%s.%s",__FUNCTION__,"toggle");
		lib3270_set_toggle(window->host,prop_id - v3270_properties.type.toggle, (int) g_value_get_boolean (value));

	}
	else
	{
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}

 }

 static void v3270_get_property(GObject *object,guint prop_id, GValue *value, GParamSpec *pspec)
 {
	v3270  *window = GTK_V3270(object);

 	debug("%s(%u,%s)",__FUNCTION__,prop_id,g_param_spec_get_name(pspec));

 	if(prop_id >= v3270_properties.type.str)
	{
		const LIB3270_STRING_PROPERTY * prop = (lib3270_get_string_properties_list()+(prop_id - v3270_properties.type.str));
		debug("%s.%s.%s",__FUNCTION__,"string",prop->name);

		if(prop->get)
			g_value_set_string(value,prop->get(window->host));

	}
	else if(prop_id >= v3270_properties.type.integer)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_int_properties_list()+(prop_id - v3270_properties.type.integer));
		debug("%s.%s.%s",__FUNCTION__,"integer",prop->name);

		if(prop->get)
			g_value_set_int(value,prop->get(window->host));

	}
	else if(prop_id >= v3270_properties.type.boolean)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_boolean_properties_list()+(prop_id - v3270_properties.type.boolean));
		debug("%s.%s.%s",__FUNCTION__,"boolean",prop->name);

		if(prop->get)
			g_value_set_boolean(value,prop->get(window->host) != 0 ? TRUE : FALSE);

	}
 	else if(prop_id >= v3270_properties.type.toggle)
	{
		debug("%s.%s.%s",__FUNCTION__,"toggle",lib3270_get_toggle_name(prop_id - v3270_properties.type.toggle));
		g_value_set_boolean(value,lib3270_get_toggle(window->host,prop_id - v3270_properties.type.toggle) ? TRUE : FALSE );

	}
	else
	{
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}

 }

 void v3270_install_property(GObjectClass *oclass, guint property_id, GParamSpec *pspec)
 {
 	static const struct
 	{
 		const char	*name;
 		GParamSpec	**prop;
 	} properties[] = {
 		{ "connected",		&v3270_properties.online	},
 		{ "luname",			&v3270_properties.luname	},
 		{ "model",			&v3270_properties.model		},
 		{ "has-selection",	&v3270_properties.selection	}
 	};

 	size_t ix;

 	debug("Property %s=%u",g_param_spec_get_name(pspec),(unsigned int) property_id);
	g_object_class_install_property(oclass, property_id, pspec);

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++)
	{
		if(!g_ascii_strcasecmp(properties[ix].name,g_param_spec_get_name(pspec)))
		{
			debug("Property \"%s\" is special",g_param_spec_get_name(pspec));
			*properties[ix].prop = pspec;
			break;
		}
	}

 }

 void v3270_init_properties(GObjectClass * gobject_class)
 {
 	size_t		  ix;
 	GParamSpec	* spec;

 	debug("%s",__FUNCTION__);

 	memset(&v3270_properties,0,sizeof(v3270_properties));
 	v3270_properties.count = LIB3270_TOGGLE_COUNT;
	v3270_properties.type.toggle = PROP_BEGIN;

	gobject_class->set_property = v3270_set_property;
	gobject_class->get_property = v3270_get_property;

	// Get property tables
	const LIB3270_INT_PROPERTY		* bool_props	= lib3270_get_boolean_properties_list();
	const LIB3270_INT_PROPERTY		* int_props		= lib3270_get_int_properties_list();
	const LIB3270_STRING_PROPERTY	* str_props		= lib3270_get_string_properties_list();

	v3270_properties.type.boolean = v3270_properties.count + PROP_BEGIN;
	for(ix = 0; bool_props[ix].name; ix++)
	{
		v3270_properties.count++;
	}

	v3270_properties.type.integer = v3270_properties.count + PROP_BEGIN;
	for(ix = 0; int_props[ix].name; ix++)
	{
		v3270_properties.count++;
	}

	v3270_properties.type.str = v3270_properties.count + PROP_BEGIN;
	for(ix = 0; str_props[ix].name; ix++)
	{
		v3270_properties.count++;
	}

	debug("Creating %u properties", (unsigned int) v3270_properties.count);

	// Creating toggle properties.
	for(ix = 0; ix < LIB3270_TOGGLE_COUNT; ix++)
	{
		debug("Property %u=%s (Toggle)",(unsigned int) v3270_properties.type.toggle + ix, lib3270_get_toggle_name(ix));
		v3270_properties.toggle[ix] = g_param_spec_boolean(lib3270_get_toggle_name(ix),lib3270_get_toggle_name(ix),lib3270_get_toggle_description(ix),FALSE,G_PARAM_WRITABLE|G_PARAM_READABLE);
		v3270_install_property(gobject_class, v3270_properties.type.toggle + ix, v3270_properties.toggle[ix]);
	}


	// Creating boolean properties.
	for(ix = 0; bool_props[ix].name; ix++)
	{
		debug("Property %u=%s (Boolean)",(unsigned int) v3270_properties.type.boolean + ix, bool_props[ix].name);
		spec = g_param_spec_boolean(bool_props[ix].name, bool_props[ix].name, bool_props[ix].description, FALSE,(bool_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE)));
		v3270_install_property(gobject_class, v3270_properties.type.boolean + ix, spec);

	}

	// Creating integer properties.
	for(ix = 0; int_props[ix].name; ix++)
	{
		debug("Property %u=%s (Integer)",(unsigned int) v3270_properties.type.integer + ix, int_props[ix].name);

		spec = g_param_spec_int(
			int_props[ix].name,
			int_props[ix].name,
			int_props[ix].description,
			0,			// Minimo
			INT_MAX,	// Máximo
			0,			// Default
			(int_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE))
		);

		v3270_install_property(gobject_class, v3270_properties.type.integer + ix, spec);

	}

	// Creating string properties.
	for(ix = 0; str_props[ix].name; ix++)
	{
		debug("Property %u=%s (String)",(unsigned int) v3270_properties.type.str + ix, str_props[ix].name);
		spec = g_param_spec_string(str_props[ix].name, str_props[ix].name, str_props[ix].description, FALSE,(str_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE)));
		v3270_install_property(gobject_class, v3270_properties.type.str + ix, spec);

	}

 }

 LIB3270_EXPORT void v3270_set_auto_disconnect(GtkWidget *widget, guint minutes)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
 	GTK_V3270(widget)->activity.disconnect = minutes;
 }

 LIB3270_EXPORT guint v3270_get_auto_disconnect(GtkWidget *widget)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),0);
 	return GTK_V3270(widget)->activity.disconnect;
 }

LIB3270_EXPORT gboolean v3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_get_toggle(GTK_V3270(widget)->host,ix) ? TRUE : FALSE;

	return FALSE;
}

LIB3270_EXPORT gboolean	v3270_set_toggle(GtkWidget *widget, LIB3270_TOGGLE ix, gboolean state)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_set_toggle(GTK_V3270(widget)->host,ix,state ? 1 : 0) ? TRUE : FALSE;

	return FALSE;

}

