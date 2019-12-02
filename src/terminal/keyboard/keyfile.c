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

 #include <config.h>
 #include <gtk/gtk.h>
 #include <terminal.h>
 #include "private.h"

 //#include <v3270/actions.h>

 struct Args
 {
 	GKeyFile	* key_file;
	const gchar * group_name;
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void save_accelerator(const V3270Accelerator * accel, const char *keys, gpointer ptr)
 {
 	const gchar * key = v3270_accelerator_get_name(accel);
 	if(!key)
		return;

	debug("%s=%s",v3270_accelerator_get_name(accel),keys);

	g_key_file_set_string(
		((struct Args *) ptr)->key_file,
		((struct Args *) ptr)->group_name,
		key,
		(keys ? keys : "")
	);

 }

 void v3270_accelerator_map_to_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name)
 {
	struct Args args = { key_file, group_name };

	g_key_file_remove_group(key_file,group_name,NULL);
	v3270_accelerator_map_foreach(GTK_WIDGET(widget),save_accelerator,&args);

 }

 void v3270_accelerator_map_set_entry(v3270 *terminal, const gchar *name, const gchar *keys)
 {
	V3270Accelerator * accel = NULL;

	// Find accelerator by name
	{
		GSList * ix = terminal->accelerators;

		while(ix)
		{
			const gchar *accel_name = v3270_accelerator_get_name((V3270Accelerator *) ix->data);

			if(accel_name && !g_ascii_strcasecmp(accel_name,name))
			{
				// It's the same name, steal it.
				if(!accel)
					accel = (V3270Accelerator *) ix->data;
				else
					g_free(ix->data);

				terminal->accelerators = g_slist_remove_link(terminal->accelerators, ix);
				ix = terminal->accelerators;

			}
			else
			{
				// Not the same name, get the next one.
				ix = g_slist_next(ix);
			}
		}

	}

	if(!accel)
	{
		debug("Can't find accelerator \"%s\"",name);
		g_warning("Can't find accelerator \"%s\"",name);
		return;
	}

	debug("Recreating accelerators for action \"%s\"",v3270_accelerator_get_name(accel));

	{
		size_t ix;
		gchar ** keycodes = g_strsplit(keys,",",-1);

		for(ix=0;keycodes[ix];ix++)
		{
			V3270Accelerator * acc = v3270_accelerator_copy(accel);
			gtk_accelerator_parse(keycodes[ix],&acc->key,&acc->mods);
			terminal->accelerators = g_slist_prepend(terminal->accelerators,acc);
		}

		g_strfreev(keycodes);
	}

	g_free(accel);
 }

 gboolean v3270_accelerator_map_load_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

 	if(!group_name)
		group_name = "accelerators";

 	v3270 * terminal = GTK_V3270(widget);

 	gchar **keys = g_key_file_get_keys(key_file,group_name,NULL,NULL);

	if(!keys)
		return FALSE;

	size_t ix;
	for(ix = 0; keys[ix]; ix++)
	{
		g_autofree gchar * value = g_key_file_get_string(key_file, group_name, keys[ix],NULL);

		if(value)
			v3270_accelerator_map_set_entry(terminal,keys[ix],value);

	}

	g_strfreev(keys);

	v3270_accelerator_map_sort(terminal);

 	return TRUE;
 }
