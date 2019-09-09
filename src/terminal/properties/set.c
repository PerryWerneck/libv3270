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

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);

		}

	}


 }

