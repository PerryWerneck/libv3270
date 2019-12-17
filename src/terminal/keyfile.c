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
 #include <terminal.h>
 #include <internals.h>
 #include <v3270/settings.h>
 #include <lib3270/toggle.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void save_by_pspec(GtkWidget *widget, GParamSpec *pspec, GKeyFile *key_file, const gchar *group_name)
 {
 	if(!pspec)
	{
		g_warning("Invalid property");
		return;
	}

 	const gchar * name = g_param_spec_get_name(pspec);
	GValue value = G_VALUE_INIT;

	g_value_init(&value, pspec->value_type);
	g_object_get_property(G_OBJECT(widget),name,&value);

	switch(pspec->value_type)
	{
	case G_TYPE_STRING:
		{
			const gchar * current = g_value_get_string(&value);

//			debug("%s=%s (default: %s)",name,current,G_PARAM_SPEC_STRING(pspec)->default_value);

			if(current && strcmp(current,G_PARAM_SPEC_STRING(pspec)->default_value ? G_PARAM_SPEC_STRING(pspec)->default_value : ""))
			{
				g_key_file_set_string(
					key_file,
					group_name,
					name,
					current
				);
			}
			else
			{
				g_key_file_remove_key(
					key_file,
					group_name,
					name,
					NULL
				);
			}
		}
		break;

	case G_TYPE_BOOLEAN:
		{
			gboolean current = g_value_get_boolean(&value);

//			debug("%s=%s (default: %s)",name,current ? "ON" : "OFF" ,G_PARAM_SPEC_BOOLEAN(pspec)->default_value ? "ON" : "OFF");

			if(current != G_PARAM_SPEC_BOOLEAN(pspec)->default_value)
			{
				g_key_file_set_boolean(
					key_file,
					group_name,
					name,
					current
				);
			}
			else
			{
				g_key_file_remove_key(
					key_file,
					group_name,
					name,
					NULL
				);
			}
		}
		break;

	case G_TYPE_INT:
		{
			gint current = g_value_get_int(&value);

			if(current != G_PARAM_SPEC_INT(pspec)->default_value)
			{
				g_key_file_set_integer(
					key_file,
					group_name,
					name,
					current
				);
			}
			else
			{
				g_key_file_remove_key(
					key_file,
					group_name,
					name,
					NULL
				);
			}

		}
		break;

	case G_TYPE_UINT:
		{
			guint current = (gint) g_value_get_uint(&value);

			if(current != G_PARAM_SPEC_UINT(pspec)->default_value)
			{
				g_key_file_set_integer(
					key_file,
					group_name,
					name,
					(gint) current
				);
			}
			else
			{
				g_key_file_remove_key(
					key_file,
					group_name,
					name,
					NULL
				);
			}

		}
		break;

	default:
		lib3270_write_trace(v3270_get_session(widget),"%s has an unexpected value type\n",name);

	}

	g_value_unset(&value);

 }

 static void load_by_pspec(GtkWidget *widget, GParamSpec *pspec, GKeyFile *key_file, const gchar *group_name)
 {
 	if(!pspec)
	{
		g_warning("Invalid property");
		return;
	}

 	const gchar * name = g_param_spec_get_name(pspec);
 	GError		* error = NULL;

	if(!g_key_file_has_key(key_file,group_name,name,&error))
	{
		if(error)
		{
			g_message("%s::%s: %s",group_name,name,error->message);
			g_error_free(error);
		}
		return;
	}

	debug("%s(%s)",__FUNCTION__,name);

	GValue value = G_VALUE_INIT;
	g_value_init(&value, pspec->value_type);

	switch(pspec->value_type)
	{
	case G_TYPE_STRING:
		g_value_take_string(&value, g_key_file_get_string(key_file,group_name,name,NULL));
		break;

	case G_TYPE_BOOLEAN:
		g_value_set_boolean(&value, g_key_file_get_boolean(key_file,group_name,name,NULL));
		break;

	case G_TYPE_INT:
		g_value_set_int(&value, g_key_file_get_integer(key_file,group_name,name,NULL));
		break;

	case G_TYPE_UINT:
		g_value_set_uint(&value, (guint) g_key_file_get_integer(key_file,group_name,name,NULL));
		break;

	default:
		lib3270_write_trace(v3270_get_session(widget),"%s has an unexpected value type\n",name);
		g_value_unset(&value);
		return;

	}

	g_object_set_property(G_OBJECT(widget),name,&value);
	g_value_unset(&value);

 }

 /// @brief Reads the terminal settings from the group group_name in key_file.
 LIB3270_EXPORT void v3270_to_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name)
 {
	g_return_if_fail(GTK_IS_V3270(widget));

	size_t		  ix;
	v3270 		* terminal	= GTK_V3270(widget);
	v3270Class	* klass		= GTK_V3270_GET_CLASS(widget);

	// Save Toggles
	for(ix = 0; ix < G_N_ELEMENTS(klass->properties.toggle); ix++)
		save_by_pspec(widget,klass->properties.toggle[ix],key_file,group_name);

	// Save V3270 Responses
	for(ix = 0; ix < G_N_ELEMENTS(terminal->responses); ix++)
		save_by_pspec(widget,klass->responses[ix],key_file,group_name);

	// Save V3270 properties
	for(ix = 0; ix < V3270_SETTING_COUNT; ix++)
		save_by_pspec(widget,klass->properties.settings[ix],key_file,group_name);

 }

 /// @brief This function adds the terminal settings from widget to key_file.
 LIB3270_EXPORT gboolean v3270_load_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	size_t		  ix;

	v3270 		* terminal	= GTK_V3270(widget);
	v3270Class	* klass		= GTK_V3270_GET_CLASS(widget);

	if(!group_name)
		group_name = "terminal";

	g_object_freeze_notify(G_OBJECT(widget));
	terminal->freeze = 1;

	// Load Toggles
	for(ix = 0; ix < G_N_ELEMENTS(klass->properties.toggle); ix++)
		load_by_pspec(widget,klass->properties.toggle[ix],key_file,group_name);

	// Load V3270 Responses
	for(ix = 0; ix < G_N_ELEMENTS(terminal->responses); ix++)
		load_by_pspec(widget,klass->responses[ix],key_file,group_name);

	// Load V3270 properties
	for(ix = 0; ix < V3270_SETTING_COUNT; ix++)
		load_by_pspec(widget,klass->properties.settings[ix],key_file,group_name);

	g_object_thaw_notify(G_OBJECT(widget));
	terminal->freeze = 0;

	return TRUE;
 }
