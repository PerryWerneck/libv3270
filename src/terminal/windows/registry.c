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
 #include <windows.h>
 #include <terminal.h>
 #include <internals.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>
 #include <lib3270/toggle.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void save_by_pspec(GtkWidget *widget, GParamSpec *pspec, HKEY hKey)
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

			debug("%s=%s (default: %s)",name,current,G_PARAM_SPEC_STRING(pspec)->default_value);

			if(current && strcmp(current,G_PARAM_SPEC_STRING(pspec)->default_value ? G_PARAM_SPEC_STRING(pspec)->default_value : ""))
			{
				RegSetValueEx(hKey,name,0,REG_SZ,(const BYTE *) current,strlen(current)+1);
			}
			else
			{
				RegDeleteValue(hKey,name);
			}
		}
		break;

	case G_TYPE_BOOLEAN:
		{
			gboolean current = g_value_get_boolean(&value);

			if(current != G_PARAM_SPEC_BOOLEAN(pspec)->default_value)
			{
			 	DWORD dw = (DWORD) current;
				RegSetValueEx(hKey, name, 0, REG_DWORD,(const BYTE *) &dw,sizeof(dw));
			}
			else
			{
				RegDeleteValue(hKey,name);
			}
		}
		break;

	case G_TYPE_INT:
		{
			gint current = g_value_get_int(&value);

			if(current != G_PARAM_SPEC_INT(pspec)->default_value)
			{
			 	DWORD dw = (DWORD) current;
				RegSetValueEx(hKey, name, 0, REG_DWORD,(const BYTE *) &dw,sizeof(dw));
			}
			else
			{
				RegDeleteValue(hKey,name);
			}

		}
		break;

	case G_TYPE_UINT:
		{
			guint current = (gint) g_value_get_uint(&value);

			if(current != G_PARAM_SPEC_UINT(pspec)->default_value)
			{
			 	DWORD dw = (DWORD) current;
				RegSetValueEx(hKey, name, 0, REG_DWORD,(const BYTE *) &dw,sizeof(dw));
			}
			else
			{
				RegDeleteValue(hKey,name);
			}

		}
		break;

	default:
		lib3270_write_trace(v3270_get_session(widget),"%s has an unexpected value type\n",name);

	}

	g_value_unset(&value);

 }

 static void load_by_pspec(GtkWidget *widget, GParamSpec *pspec, HKEY hKey)
 {
 	if(!pspec)
	{
		g_warning("Invalid property");
		return;
	}

 	const gchar * name = g_param_spec_get_name(pspec);

	BYTE data[4097];
	unsigned long datatype;
	unsigned long datalen 	= 4096;

	memset(data,0,sizeof(data));

	if(RegQueryValueExA(hKey,name,NULL,&datatype,data,&datalen) != ERROR_SUCCESS)
		return;

	GValue value = G_VALUE_INIT;
	g_value_init(&value, pspec->value_type);
	g_object_get_property(G_OBJECT(widget),name,&value);

	switch(pspec->value_type)
	{
	case G_TYPE_STRING:
		if(datatype == REG_SZ)
			g_value_set_string(&value, (const gchar *) data);
		break;

	case G_TYPE_BOOLEAN:
		if(datatype == REG_DWORD)
			g_value_set_boolean(&value,  * ((DWORD *) data) == 0 ? FALSE : TRUE);
		break;

	case G_TYPE_INT:
		if(datatype == REG_DWORD)
			g_value_set_int(&value, (gint) * ((DWORD *) data));
		break;

	case G_TYPE_UINT:
		if(datatype == REG_DWORD)
			g_value_set_uint(&value, (guint) * ((DWORD *) data));
		break;

	default:
		lib3270_write_trace(v3270_get_session(widget),"%s has an unexpected value type\n",name);
		g_value_unset(&value);
		return;

	}

	g_object_set_property(G_OBJECT(widget),name,&value);
	g_value_unset(&value);

 }

 /// @brief Reads the terminal settings from the group group_name in registry.
 LIB3270_EXPORT void v3270_to_registry(GtkWidget *widget, HKEY hParent, const gchar *group_name)
 {
	g_return_if_fail(GTK_IS_V3270(widget));

	// Open registry
 	HKEY	hKey;
 	DWORD	disp;

	if(RegCreateKeyEx(hParent,group_name,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) != ERROR_SUCCESS)
	{
		g_warning("Can't open registry");
		return;
	}

	// Save settings
	size_t		  ix;
	GString		* str;

	v3270 		* terminal	= GTK_V3270(widget);
	v3270Class	* klass		= GTK_V3270_GET_CLASS(widget);

	// Save Toggles
	for(ix = 0; ix < G_N_ELEMENTS(klass->properties.toggle); ix++)
		save_by_pspec(widget,klass->properties.toggle[ix],hKey);

	// Save V3270 Responses
	for(ix = 0; ix < G_N_ELEMENTS(terminal->responses); ix++)
		save_by_pspec(widget,klass->responses[ix],hKey);

	// Save V3270 properties
	for(ix = 0; ix < V3270_SETTING_COUNT; ix++)
		save_by_pspec(widget,klass->properties.settings[ix],hKey);

	RegCloseKey(hKey);

 }

 /// @brief This function adds the terminal settings from widget to windows registry.
 LIB3270_EXPORT gboolean v3270_load_registry(GtkWidget *widget, HKEY hParent, const gchar *group_name)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	// Open registry
 	HKEY	hKey;
 	DWORD	disp;

	if(RegCreateKeyEx(hParent,group_name,0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&disp) != ERROR_SUCCESS)
	{
		g_warning("Can't open registry");
		return FALSE;
	}

	// Load settings.
	size_t		  ix;

	v3270 		* terminal	= GTK_V3270(widget);
	v3270Class	* klass		= GTK_V3270_GET_CLASS(widget);

	g_object_freeze_notify(G_OBJECT(widget));

	// Load Toggles
	for(ix = 0; ix < G_N_ELEMENTS(klass->properties.toggle); ix++)
		load_by_pspec(widget,klass->properties.toggle[ix],hKey);

	// Load V3270 Responses
	for(ix = 0; ix < G_N_ELEMENTS(terminal->responses); ix++)
		load_by_pspec(widget,klass->responses[ix],hKey);

	// Load V3270 properties
	for(ix = 0; ix < V3270_SETTING_COUNT; ix++)
		load_by_pspec(widget,klass->properties.settings[ix],hKey);

	// Load V3270 colors
	// v3270_set_colors(widget,g_key_file_get_string(key_file,group_name,"colors",NULL));

	g_object_thaw_notify(G_OBJECT(widget));

	RegCloseKey(hKey);

	return TRUE;
 }
