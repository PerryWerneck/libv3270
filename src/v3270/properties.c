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

/*--[ Globals ]--------------------------------------------------------------------------------------*/

//  GParamSpec * v3270_properties[PROP_LAST]		= { 0 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
 {
	v3270  *window = GTK_V3270(object);

 	debug("%s(%u,%s)",__FUNCTION__,prop_id,g_param_spec_get_name(pspec));

 	if(prop_id < v3270_properties.type.toggle)
	{
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
 	else if(prop_id >= v3270_properties.type.str)
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

	}
	else
	{
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}

 	/*

	switch (prop_id)
	{
	case PROP_MODEL:
		lib3270_set_model(window->host,g_value_get_string(value));
		break;

	case PROP_AUTO_DISCONNECT:
		v3270_set_auto_disconnect(GTK_WIDGET(object),g_value_get_uint(value));
		break;

	case PROP_URL:
		v3270_set_url(GTK_WIDGET(object),g_value_get_string(value));
		break;

	case PROP_SESSION_NAME:
		v3270_set_session_name(GTK_WIDGET(object),g_value_get_string(value));
		break;

	default:
		if(prop_id < (PROP_TOGGLE + LIB3270_TOGGLE_COUNT))
		{
			lib3270_set_toggle(window->host,prop_id - PROP_TOGGLE, (int) g_value_get_boolean (value));
			return;
		}
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
	*/

 }

 static void v3270_get_property(GObject *object,guint prop_id, GValue *value, GParamSpec *pspec)
 {
	v3270  *window = GTK_V3270(object);

 	debug("%s(%u,%s)",__FUNCTION__,prop_id,g_param_spec_get_name(pspec));

 	if(prop_id < v3270_properties.type.toggle)
	{
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
 	else if(prop_id >= v3270_properties.type.str)
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


	}
	else if(prop_id >= v3270_properties.type.boolean)
	{
		const LIB3270_INT_PROPERTY * prop = (lib3270_get_boolean_properties_list()+(prop_id - v3270_properties.type.boolean));
		debug("%s.%s.%s",__FUNCTION__,"boolean",prop->name);


	}
 	else if(prop_id >= v3270_properties.type.toggle)
	{
		debug("%s.%s",__FUNCTION__,"toggle");

	}
	else
	{
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}

 	/*
	v3270  *window = GTK_V3270(object);

	switch (prop_id)
	{
	case PROP_MODEL:
		g_value_set_string(value,lib3270_get_model(window->host));
		break;

	case PROP_AUTO_DISCONNECT:
		g_value_set_uint(value,v3270_get_auto_disconnect(GTK_WIDGET(object)));
		break;

	case PROP_LUNAME:
		g_value_set_string(value,lib3270_get_luname(window->host));
		break;

	case PROP_ONLINE:
		g_value_set_boolean(value,lib3270_is_connected(window->host) ? TRUE : FALSE );
		break;

	case PROP_SELECTION:
		g_value_set_boolean(value,lib3270_has_selection(window->host) ? TRUE : FALSE );
		break;

	case PROP_URL:
		g_value_set_string(value,lib3270_get_url(window->host));
		break;

	case PROP_SESSION_NAME:
		g_value_set_string(value,v3270_get_session_name(GTK_WIDGET(object)));
		break;

	default:
		if(prop_id < (PROP_TOGGLE + LIB3270_TOGGLE_COUNT))
		{
			g_value_set_boolean(value,lib3270_get_toggle(window->host,prop_id - PROP_TOGGLE) ? TRUE : FALSE );
			return;
		}
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
	*/
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
		spec = g_param_spec_boolean(lib3270_get_toggle_name(ix),lib3270_get_toggle_name(ix),lib3270_get_toggle_description(ix),FALSE,G_PARAM_WRITABLE|G_PARAM_READABLE);
		g_object_class_install_property(gobject_class, v3270_properties.type.toggle + ix, spec);
	}


	// Creating boolean properties.
	for(ix = 0; bool_props[ix].name; ix++)
	{
		debug("Property %u=%s (Boolean)",(unsigned int) v3270_properties.type.boolean + ix, bool_props[ix].name);
		spec = g_param_spec_boolean(bool_props[ix].name, bool_props[ix].name, bool_props[ix].description, FALSE,(bool_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE)));
		g_object_class_install_property(gobject_class, v3270_properties.type.boolean + ix, spec);

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

		g_object_class_install_property(gobject_class, v3270_properties.type.integer + ix, spec);

	}

	// Creating string properties.
	for(ix = 0; str_props[ix].name; ix++)
	{
		debug("Property %u=%s (String)",(unsigned int) v3270_properties.type.str + ix, str_props[ix].name);
		spec = g_param_spec_string(str_props[ix].name, str_props[ix].name, str_props[ix].description, FALSE,(str_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE)));
		g_object_class_install_property(gobject_class, v3270_properties.type.str + ix, spec);

	}


	/*
	v3270_properties[PROP_ONLINE] = g_param_spec_boolean(
					"online",
					"online",
					"True if is online",
					FALSE,G_PARAM_READABLE);
	g_object_class_install_property(gobject_class,PROP_ONLINE,v3270_properties[PROP_ONLINE]);

	v3270_properties[PROP_SELECTION] = g_param_spec_boolean(
					"selection",
					"selection",
					"True on selected area",
					FALSE,G_PARAM_READABLE);
	g_object_class_install_property(gobject_class,PROP_SELECTION,v3270_properties[PROP_SELECTION]);

	v3270_properties[PROP_MODEL] = g_param_spec_string(
					"model",
					"model",
					"The model of 3270 display to be emulated",
					NULL,
					G_PARAM_READABLE|G_PARAM_WRITABLE);

	g_object_class_install_property(gobject_class,PROP_MODEL,v3270_properties[PROP_MODEL]);

	v3270_properties[PROP_LUNAME] = g_param_spec_string(
					"luname",
					"luname",
					"The logical Unit (LU) name",
					NULL,
					G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_LUNAME,v3270_properties[PROP_LUNAME]);

	v3270_properties[PROP_AUTO_DISCONNECT] = g_param_spec_uint(
					"auto_disconnect",
					"auto_disconnect",
					"Minutes to disconnect when idle",
					0,			// Minimo
					3600,		// Máximo
					0,			// Default
					G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_AUTO_DISCONNECT,v3270_properties[PROP_AUTO_DISCONNECT]);

	v3270_properties[PROP_URL] = g_param_spec_string(
					"url",
					"url",
					"Host URL",
					NULL,
					G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_AUTO_DISCONNECT,v3270_properties[PROP_URL]);

	v3270_properties[PROP_SESSION_NAME] = g_param_spec_string(
					"session_name",
					"session_name",
					"The TN3270 Session Name",
					g_get_application_name(),
					G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_SESSION_NAME,v3270_properties[PROP_SESSION_NAME]);

	// Toggle properties
	int f;

	for(f=0;f<LIB3270_TOGGLE_COUNT;f++)
	{
		v3270_properties[PROP_TOGGLE+f] = g_param_spec_boolean(lib3270_get_toggle_name(f),lib3270_get_toggle_name(f),lib3270_get_toggle_description(f),FALSE,G_PARAM_WRITABLE|G_PARAM_READABLE);
		g_object_class_install_property(gobject_class,PROP_TOGGLE+f,v3270_properties[PROP_TOGGLE+f]);
	}
	debug("%s",__FUNCTION__);
	*/
 }

 void v3270_set_auto_disconnect(GtkWidget *widget, guint minutes)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
 	GTK_V3270(widget)->activity.disconnect = minutes;
 }

 guint v3270_get_auto_disconnect(GtkWidget *widget)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),0);
 	return GTK_V3270(widget)->activity.disconnect;
 }
