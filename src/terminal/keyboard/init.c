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
 #include <gdk/gdkkeysyms-compat.h>
 #include <v3270/actions.h>
 #include <lib3270/toggle.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

 #ifndef GDK_ALT_MASK
	#define GDK_ALT_MASK GDK_MOD1_MASK
 #endif

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static int fire_lib3270_action(GtkWidget *widget, const LIB3270_ACTION * action)
 {
 	int rc = EPERM;

 	H3270 *hSession = v3270_get_session(widget);

    debug("%s(%s)",__FUNCTION__,action->name);

    if(action->activatable(hSession))
		rc = action->activate(hSession);

	debug("%s(%s)=%d %s",__FUNCTION__,action->name,rc,strerror(rc));
	return rc;

 }

 static int fire_lib3270_toggle(GtkWidget *widget, const LIB3270_TOGGLE * action)
 {
    debug("%s(%s)",__FUNCTION__,action->name);
 	return lib3270_toggle(v3270_get_session(widget),action->id);
 }

 int fire_keypad_action(GtkWidget *widget, const struct _v3270_action * action)
 {
 	int rc = 0;
 	debug("%s",__FUNCTION__);

	if(v3270_get_toggle(widget,LIB3270_TOGGLE_KP_ALTERNATIVE))
	{
		if(action->key == GDK_KP_Add)
			rc = lib3270_nextfield(GTK_V3270(widget)->host);
		else
			rc = lib3270_previousfield(GTK_V3270(widget)->host);
	}
	else
	{
		v3270_set_string(widget, action->key == GDK_KP_Add ? "+" : "-");
	}

	return rc;

 }

 void v3270_init_accelerators(v3270 *widget)
 {
 	size_t ix;

	// Create accelerators for lib3270 actions.
	{
		const LIB3270_ACTION * actions = lib3270_get_actions();

		for(ix = 0; actions[ix].name; ix++)
		{
			if(actions[ix].keys && *actions[ix].keys)
			{
				size_t key;

				gchar ** keys = g_strsplit(actions[ix].keys,",",-1);

				for(key = 0; keys[key]; key++)
				{

					V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

					accelerator->type 		= V3270_ACCELERATOR_TYPE_LIB3270_ACTION;
					accelerator->arg 		= (gconstpointer) &actions[ix];
					accelerator->activate	= G_CALLBACK(fire_lib3270_action);

//					debug("%s=%s",actions[ix].name,keys[key]);

					gtk_accelerator_parse(keys[key],&accelerator->key,&accelerator->mods);

					widget->accelerators = g_slist_prepend(widget->accelerators,accelerator);

				}

				g_strfreev(keys);

			}
			else
			{
				V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

				accelerator->type = V3270_ACCELERATOR_TYPE_LIB3270_ACTION;
				accelerator->arg  = (gconstpointer) &actions[ix];
				accelerator->activate = G_CALLBACK(fire_lib3270_action);
				widget->accelerators = g_slist_prepend(widget->accelerators,accelerator);

			}

		}

	}

	// Create accelerators for lib3270 toggles.
	{
		const LIB3270_TOGGLE * toggles = lib3270_get_toggles();
		size_t ix;

		for(ix = 0; toggles[ix].name; ix++)
		{
			V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

			accelerator->type = V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE;
			accelerator->arg = (gconstpointer) &toggles[ix];
			accelerator->activate = G_CALLBACK(fire_lib3270_toggle);

			if(toggles[ix].key)
				gtk_accelerator_parse(toggles[ix].key,&accelerator->key,&accelerator->mods);

			widget->accelerators = g_slist_prepend(widget->accelerators,accelerator);

		}

	}

	// Create accelerators for internal actions.
	{
        size_t ix;

		const V3270_ACTION * actions = v3270_get_actions();

        for(ix = 0 ; actions[ix].name; ix++)
		{
			V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

			accelerator->type = V3270_ACCELERATOR_TYPE_INTERNAL;
			accelerator->arg = (gconstpointer) &actions[ix];
			accelerator->activate = G_CALLBACK(actions[ix].activate);
			accelerator->key = actions[ix].key;
			accelerator->mods = actions[ix].mods;

			widget->accelerators = g_slist_prepend(widget->accelerators,accelerator);

		}
	}

	v3270_accelerator_map_sort(widget);

 }

