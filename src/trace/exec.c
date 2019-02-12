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

 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/properties.h>
 #include <v3270/trace.h>

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

 static void set_property(H3270 *hSession, gchar *ptr)
 {
 	size_t ix;

 	const gchar * name = get_word(&ptr);

 	debug("property_name: \"%s\"",name);

 	// Check toggles
 	for(ix = 0; ix < (size_t) LIB3270_TOGGLE_COUNT; ix++)
	{
		debug("%s %s %d",name,lib3270_get_toggle_name((LIB3270_TOGGLE) ix),g_ascii_strcasecmp(name,lib3270_get_toggle_name((LIB3270_TOGGLE) ix)));
		if(g_ascii_strcasecmp(name,lib3270_get_toggle_name((LIB3270_TOGGLE) ix)) == 0)
		{
			lib3270_set_toggle(hSession,(LIB3270_TOGGLE) ix, 1);
			return;
		}
	}

 	// Check boolean properties
 	const LIB3270_INT_PROPERTY * iProp = lib3270_get_boolean_properties_list();
 	for(ix = 0; iProp[ix].name; ix++)
	{
		debug("%s %s %d",name,iProp[ix].name,g_ascii_strcasecmp(name,iProp[ix].name));
		if(g_ascii_strcasecmp(name,iProp[ix].name) == 0 && iProp[ix].set)
		{
			iProp[ix].set(hSession,1);
			return;
		}
	}



 }

 void v3270_exec_command(GtkWidget *widget, const gchar *text)
 {
	g_autofree gchar * cmdline = g_strdup(text);

 	g_strstrip(cmdline);

 	debug("cmdline: \"%s\"",cmdline);

 	if(g_str_has_prefix(cmdline,"set "))
	{
		set_property(v3270_get_session(widget), cmdline+3);
		return;
	}

 }
