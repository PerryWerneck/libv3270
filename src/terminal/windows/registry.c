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
 #include <winsock2.h>
 #include <windows.h>
 #include <terminal.h>
 #include <internals.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>
 #include <lib3270/toggle.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>

 static const HKEY predefined[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void save_string(HKEY hKey, const gchar *key, const gchar *value) {

	 if(value) {
		RegSetValueEx(hKey,key,0,REG_SZ,(const BYTE *) value,strlen(value)+1);
	 } else {
		RegDeleteValue(hKey,key);
	 }
 }

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

 static void load_string(HKEY hKey, const gchar *key, gchar **value) {

	if(*value) {
		g_free(*value);
		*value = NULL;
	}

	BYTE data[4097];
	unsigned long datatype;
	unsigned long datalen 	= 4096;

	if(RegQueryValueExA(hKey,key,NULL,&datatype,data,&datalen) != ERROR_SUCCESS)
		return;

	*value = g_strdup((const gchar *) data);

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

	v3270 		* terminal	= GTK_V3270(widget);
	v3270Class	* klass		= GTK_V3270_GET_CLASS(widget);

	// Save session name.
	{
		g_autofree gchar * session_name = g_strdup(terminal->session.name ? terminal->session.name : G_STRINGIFY(PRODUCT_NAME));

		gchar *ptr = strrchr(session_name,':');
		if(ptr)
			*ptr = 0;

		if(g_ascii_strcasecmp(session_name,G_STRINGIFY(PRODUCT_NAME))) {
			RegSetValueEx(hKey,"session-name",0,REG_SZ,(const BYTE *) session_name,strlen(session_name)+1);
		} else {
			RegDeleteValue(hKey,"session-name");
		}

	}

	// Save internal properties
	save_string(hKey, "selection-font-family", terminal->selection.font_family);

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

	// Load session name
	{
		BYTE data[4097];
		unsigned long datatype;
		unsigned long datalen 	= 4096;

		memset(data,0,sizeof(data));

		if(RegQueryValueExA(hKey,"session-name",NULL,&datatype,data,&datalen) == ERROR_SUCCESS) {

			v3270_set_session_name(widget, (const gchar *) data);

		}

	}

 	// Load internal properties.
	load_string(hKey, "selection-font-family", &terminal->selection.font_family);

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

 gboolean v3270_win32_open_regkey(GtkWidget *widget, HKEY *hKey, REGSAM samDesired) {

	const gchar * session_name = GTK_V3270(widget)->session.name;

	if(!session_name)
		session_name = g_get_application_name();

	size_t				  ix;
	g_autofree gchar	* path = g_strjoin("\\software\\",session_name,NULL);

	// Remove delimiters
	static const gchar	  delim[] = { ':', '.', '?' };
	for(ix = 0; ix < G_N_ELEMENTS(delim); ix++) {

		gchar * p = strchr(path,delim[ix]);
		if(p)
			*p = 0;
	}

	for(ix=0;ix < G_N_ELEMENTS(predefined);ix++) {

		if(RegOpenKeyEx(predefined[ix],path,0,samDesired,hKey) == ERROR_SUCCESS)
			return TRUE;

	}

	return FALSE;

 }

