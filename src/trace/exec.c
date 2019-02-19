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

 #include <gtk/gtk.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <stdlib.h>

 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/properties.h>
 #include <lib3270/actions.h>
 #include <v3270.h>
 #include <v3270/trace.h>
 #include <internals.h>

 static const gchar * get_word(gchar **ptr)
 {
	gchar *rc = g_strstrip(*ptr);
	gchar *next = rc;

	while(*next && !g_ascii_isspace(*next))
		next++;

	*(next++) = 0;
	*ptr = g_strstrip(next);

	debug("next: \"%s\"",*ptr);


 	return rc;
 }

 static int set_property(H3270 *hSession, const gchar *name, const gchar * value)
 {
 	size_t ix;

 	debug("%s=%s",name,value);

 	// Check toggles
 	for(ix = 0; ix < (size_t) LIB3270_TOGGLE_COUNT; ix++)
	{
		if(g_ascii_strcasecmp(name,lib3270_get_toggle_name((LIB3270_TOGGLE) ix)) == 0)
			return lib3270_set_toggle(hSession,(LIB3270_TOGGLE) ix, atoi(value));

	}

 	// Check boolean properties
 	const LIB3270_INT_PROPERTY * bProp = lib3270_get_boolean_properties_list();
 	for(ix = 0; bProp[ix].name; ix++)
	{
		if(g_ascii_strcasecmp(name,bProp[ix].name) == 0 && bProp[ix].set)
			return bProp[ix].set(hSession,atoi(value));

	}

	// Check integer properties
 	const LIB3270_INT_PROPERTY * iProp = lib3270_get_int_properties_list();
 	for(ix = 0; iProp[ix].name; ix++)
	{
		if(g_ascii_strcasecmp(name,iProp[ix].name) == 0 && iProp[ix].set)
			return iProp[ix].set(hSession,atoi(value));

	}

	// Check string properties
	const LIB3270_STRING_PROPERTY * sProp = lib3270_get_string_properties_list();
 	for(ix = 0; sProp[ix].name; ix++)
	{
		if(g_ascii_strcasecmp(name,sProp[ix].name) == 0 && sProp[ix].set)
			return sProp[ix].set(hSession,value);

	}

	return errno = ENOENT;

 }

 int v3270_exec_command(GtkWidget *widget, const gchar *text)
 {
 	size_t ix;

 	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

 	H3270 *hSession = v3270_get_session(widget);
	g_autofree gchar * cmdline = g_strdup(text);

 	g_strstrip(cmdline);

 	debug("cmdline: \"%s\"",cmdline);

 	if(g_str_has_prefix(cmdline,"connect"))
	{
		return lib3270_reconnect(hSession,0);
	}

 	if(g_str_has_prefix(cmdline,"disconnect"))
	{
		return lib3270_disconnect(hSession);
	}

 	if(g_str_has_prefix(cmdline,"set"))
	{
		gchar *txtptr = cmdline+3;
		const gchar * name = get_word(&txtptr);
		g_strstrip(txtptr);
		return set_property(hSession,name,(*txtptr ? txtptr : "1"));
	}

 	if(g_str_has_prefix(cmdline,"reset"))
	{
		gchar *txtptr = cmdline+3;
		const gchar * name = get_word(&txtptr);
		g_strstrip(txtptr);
		return set_property(hSession,name,(*txtptr ? txtptr : "0"));
	}

	gchar * sep = strchr(cmdline,'=');
	if(sep)
	{
		*(sep++) = 0;
		return set_property(hSession,g_strstrip(cmdline),g_strstrip(sep));
	}
	else
	{
		// Check for lib3270 actions.
		const LIB3270_ACTION_ENTRY *actions = lib3270_get_action_table();

		for(ix=0; actions[ix].name; ix++)
		{
			if(!g_ascii_strcasecmp(actions[ix].name,cmdline))
			{
				lib3270_trace_event(hSession,"Action: %s\n",actions[ix].name);
				return actions[ix].call(hSession);
			}

		}

	}

	return errno = ENOENT;
 }
