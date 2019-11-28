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
 #include <errno.h>
 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/actions.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static gint compare_func(const V3270Accelerator *a, const V3270Accelerator *b)
 {
 	if(a->activate != b->activate)
		return a->activate - b->activate;
	return a->arg - b->arg;
 }

 void v3270_accelerator_map_reset(GtkWidget *widget)
 {
 	v3270 * terminal = GTK_V3270(widget);

 	if(terminal)
	{
		if(terminal->accelerators)
		{
			g_slist_free_full(terminal->accelerators,g_free);
			terminal->accelerators = NULL;
		}
		v3270_init_accelerators(terminal);
	}

 }

 void v3270_accelerator_map_sort(v3270 *widget)
 {
	widget->accelerators = g_slist_sort(widget->accelerators, (GCompareFunc) compare_func);
 }

 gboolean v3270_accelerator_compare(const V3270Accelerator * accell, const guint keyval, const GdkModifierType mods)
 {
 	return accell->key == keyval && accell->mods == mods;
 }

 void v3270_accelerator_activate(const V3270Accelerator * acel, GtkWidget *terminal)
 {
 	int rc = ((int (*)(GtkWidget *, const void *)) acel->activate)(terminal,acel->arg);

 	if(rc)
		gdk_display_beep(gdk_display_get_default());

 }

 const V3270Accelerator * v3270_get_accelerator(GtkWidget *widget, guint keyval, GdkModifierType state)
 {
	GSList * ix;

	for(ix = GTK_V3270(widget)->accelerators; ix; ix = g_slist_next(ix))
	{
		if(v3270_accelerator_compare((V3270Accelerator *) ix->data, keyval, state))
			return (V3270Accelerator *) ix->data;
	}

	return NULL;

 }

 const gchar * v3270_accelerator_get_description(const V3270Accelerator * accel)
 {
	switch(accel->type)
	{
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:
		return gettext(((LIB3270_ACTION *) accel->arg)->summary);

	// case V3270_ACCELERATOR_TYPE_INTERNAL:
	// case V3270_ACCELERATOR_TYPE_GTK_ACTION:

	}

	return NULL;
 }

 const gchar * v3270_accelerator_get_name(const V3270Accelerator * accel)
 {
	switch(accel->type)
	{
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:
		return ((LIB3270_ACTION *) accel->arg)->name;

	case V3270_ACCELERATOR_TYPE_INTERNAL:
		return ((struct InternalAction *) accel->arg)->name;

	case V3270_ACCELERATOR_TYPE_CUSTOM:
		return ((V3270CustomAccelerator *) accel)->name;

	}

	return NULL;
 }

 void v3270_accelerator_map_foreach(GtkWidget *widget,void (*call)(const V3270Accelerator * accel, const char *keys, gpointer ptr), gpointer ptr)
 {
		GSList	* accelerator = GTK_V3270(widget)->accelerators;
		GString	* str = g_string_new("");

		while(accelerator)
		{
            V3270Accelerator * current = (V3270Accelerator *) accelerator->data;

            g_string_truncate(str,0);

            while(accelerator && (((V3270Accelerator *) accelerator->data)->activate == current->activate) && (((V3270Accelerator *) accelerator->data)->arg == current->arg))
			{
				V3270Accelerator *accel = (V3270Accelerator *) accelerator->data;
				if(accel->key)
				{

					if(str->str[0])
						g_string_append(str," ");

					if(accel->key)
					{
						g_autofree gchar * keyname = gtk_accelerator_name(accel->key,accel->mods);
						g_string_append(str,keyname);
					}

				}

				accelerator = g_slist_next(accelerator);
			}

			call(current,str->str,ptr);

		}

		g_string_free(str,FALSE);

 }

 gchar * v3270_accelerator_get_label(const V3270Accelerator * accel)
 {
    return gtk_accelerator_get_label(accel->key,accel->mods);
 }

 V3270Accelerator * v3270_accelerator_map_add_entry(GtkWidget *widget, const gchar *name, guint accel_key, GdkModifierType accel_mods, GCallback callback, gpointer data)
 {
	GSList				* ix;
 	v3270 				* terminal = GTK_V3270(widget);
	V3270Accelerator	* accel = NULL;

	// Find accel by name
	for(ix = terminal->accelerators; ix; ix = g_slist_next(ix))
	{
		const gchar * nm = v3270_accelerator_get_name((V3270Accelerator *) ix->data);
		if(nm && !g_ascii_strcasecmp(name,nm))
		{
			accel = (V3270Accelerator *) ix->data;
			break;
		}
	}

	if(!accel)
	{
		// Not found, create a custom accelerator.
		debug("%s: Adding accelerator %s",__FUNCTION__,name);

		V3270CustomAccelerator *customAccel = g_new0(V3270CustomAccelerator,1);

		customAccel->parent.type	= V3270_ACCELERATOR_TYPE_CUSTOM;
		customAccel->name			= name;

		terminal->accelerators = g_slist_prepend(terminal->accelerators,customAccel);

		accel = (V3270Accelerator *) customAccel;
	}

	accel->arg 		= data;
	accel->activate	= callback;
	accel->key		= accel_key;
	accel->mods		= accel_mods;

	// Any other accell in the same key? If yes, clear it.
	for(ix = terminal->accelerators; ix; ix = g_slist_next(ix))
	{
		V3270Accelerator * acc = (V3270Accelerator *) ix->data;
		if((acc == accel) || !(acc->key == accel->key && acc->mods == accel->mods))
			continue;

		debug("Resetting accelerator \"%s\"",v3270_accelerator_get_name(acc));

		acc->key 	= 0;
		acc->mods	= 0;

	}

	// Sort!
	v3270_accelerator_map_sort(terminal);

	return accel;

 }
