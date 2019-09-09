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

 static int set_property(GtkWidget *widget, const gchar *name, const gchar * value)
 {

	GParamSpec * spec = g_object_class_find_property(G_OBJECT_GET_CLASS(widget),name);

	if(!spec) {
		return errno = ENOENT;
	}

	GValue val = G_VALUE_INIT;

	g_value_init(&val, spec->value_type);

	switch(spec->value_type)
	{
	case G_TYPE_STRING:
		{
			g_value_set_string(&val,value);
			g_object_set_property(G_OBJECT(widget),name,&val);
		}
		break;

	case G_TYPE_BOOLEAN:
		{
			g_value_set_boolean(&val,(atoi(value) == 0 ? FALSE : TRUE));
			g_object_set_property(G_OBJECT(widget),name,&val);
		}
		break;

	case G_TYPE_INT:
		{
			g_value_set_int(&val,atoi(value));
			g_object_set_property(G_OBJECT(widget),name,&val);
		}
		break;

	case G_TYPE_UINT:
		{
			g_value_set_uint(&val,atoi(value));
			g_object_set_property(G_OBJECT(widget),name,&val);
		}
		break;

	default:
		lib3270_write_trace(v3270_get_session(widget),"%s has an unexpected value type\n",spec->name);

	}

	g_value_unset(&val);
	return 0;

 }

 static int get_property(GtkWidget *widget, const gchar *name)
 {
	GParamSpec * spec = g_object_class_find_property(G_OBJECT_GET_CLASS(widget),name);

	if(!spec) {
		return errno = ENOENT;
	}

	GValue val = G_VALUE_INIT;

	g_value_init(&val, spec->value_type);
	g_object_get_property(G_OBJECT(widget),name,&val);

	switch(spec->value_type)
	{
	case G_TYPE_STRING:
		lib3270_write_trace(v3270_get_session(widget),"%s=%s\n",spec->name,g_value_get_string(&val));
		break;

	case G_TYPE_BOOLEAN:
		lib3270_write_trace(v3270_get_session(widget),"%s=%s\n",spec->name,(g_value_get_boolean(&val) ? "true" : "false"));
		break;

	case G_TYPE_INT:
		lib3270_write_trace(v3270_get_session(widget),"%s=%d\n",spec->name,g_value_get_int(&val));
		break;

	case G_TYPE_UINT:
		lib3270_write_trace(v3270_get_session(widget),"%s=%u\n",spec->name,g_value_get_uint(&val));
		break;

	default:
		lib3270_write_trace(v3270_get_session(widget),"%s has an unexpected value type\n",spec->name);

	}

	g_value_unset(&val);
	return 0;
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

 	if(g_str_has_prefix(cmdline,"reload"))
	{
		v3270_reload(widget);
		return 0;
	}

 	if(g_str_has_prefix(cmdline,"pattern"))
	{
		lib3270_testpattern(hSession);
		return 0;
	}

 	if(g_str_has_prefix(cmdline,"copy"))
	{

		gchar * arg = cmdline+4;
		g_strstrip(arg);

		if(!(*arg && g_ascii_strcasecmp(arg,"text")))
		{
			// No argument or "text" copy text.
			v3270_copy_selection(widget, V3270_SELECT_TEXT, FALSE);
		}
		else if(!g_ascii_strcasecmp(arg,"table"))
		{
			v3270_copy_selection(widget, V3270_SELECT_TABLE, FALSE);
		}
		else if(!g_ascii_strcasecmp(arg,"append"))
		{
			v3270_append_selection(widget,FALSE);
		}
		else
		{
			return errno = EINVAL;
		}

		return 0;

	}

 	if(g_str_has_prefix(cmdline,"print"))
	{

		gchar * arg = cmdline+5;
		g_strstrip(arg);

		if(!(*arg && g_ascii_strcasecmp(arg,"all")))
		{
			// No argument or "text" copy text.
			v3270_print_all(widget,NULL);
		}
		else if(!g_ascii_strcasecmp(arg,"selected"))
		{
			v3270_print_selected(widget,NULL);
		}
		else if(!g_ascii_strcasecmp(arg,"copy"))
		{
			v3270_print_copy(widget,NULL);
		}
		else
		{
			return errno = EINVAL;
		}

		return 0;

	}

 	if(g_str_has_prefix(cmdline,"paste"))
	{

		gchar * arg = cmdline+5;
		g_strstrip(arg);

		if(!*arg)
		{
			v3270_paste(widget);
		}
		else if(!g_ascii_strcasecmp(arg,"text"))
		{
			v3270_paste_text(widget);
		}
		else
		{
			return errno = EINVAL;
		}

		return 0;
	}

	if(g_str_has_suffix(cmdline,"?"))
	{
		gchar * str = strchr(cmdline,'?');
		*str = 0;
		g_strstrip(cmdline);
		return get_property(widget,cmdline);
	}

	if(strchr(cmdline,'='))
	{
		gchar * value = strchr(cmdline,'=');
		*(value++) = 0;
		g_strstrip(cmdline);
		g_strstrip(value);
		return set_property(widget,cmdline,value);
	}

 	if(g_str_has_prefix(cmdline,"disconnect"))
	{
		return lib3270_disconnect(hSession);
	}

 	if(g_str_has_prefix(cmdline,"remap"))
	{
		gchar *txtptr = cmdline+5;
		g_strstrip(txtptr);
		v3270_set_remap_filename(widget,txtptr);
		return 0;
	}

 	if(g_str_has_prefix(cmdline,"set"))
	{
		gchar *txtptr = cmdline+3;
		const gchar * name = get_word(&txtptr);
		g_strstrip(txtptr);
		return set_property(widget,name,(*txtptr ? txtptr : "1"));
	}

 	if(g_str_has_prefix(cmdline,"get"))
	{
		gchar *txtptr = cmdline+3;
		const gchar * name = get_word(&txtptr);
		g_strstrip(txtptr);
		return get_property(widget,name);
	}

 	if(g_str_has_prefix(cmdline,"reset"))
	{
		gchar *txtptr = cmdline+3;
		const gchar * name = get_word(&txtptr);
		g_strstrip(txtptr);
		return set_property(widget,name,(*txtptr ? txtptr : "0"));
	}

	gchar * sep = strchr(cmdline,'=');
	if(sep)
	{
		*(sep++) = 0;
		return set_property(widget,g_strstrip(cmdline),g_strstrip(sep));
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
