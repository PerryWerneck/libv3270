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

 static int fire_pfkey_action(GtkWidget *widget, gpointer *id)
 {
 	debug("%s(%u)",__FUNCTION__,GPOINTER_TO_INT(id));
	return lib3270_pfkey(GTK_V3270(widget)->host,GPOINTER_TO_INT(id));
 }

 static V3270PFKeyAccelerator * v3270_pfkey_accelerator_new(int id, const gchar *keyname)
 {
	V3270PFKeyAccelerator * accelerator = g_new0(V3270PFKeyAccelerator,1);

	accelerator->parent.type = V3270_ACCELERATOR_TYPE_PFKEY;
	accelerator->parent.arg = GINT_TO_POINTER(id);

	g_autofree gchar *name = g_strdup_printf("pf%02d",id);
	debug("%s(%s)",__FUNCTION__,name);
	accelerator->name = gdk_atom_intern(name,FALSE);

	if(keyname) {

		v3270_accelerator_parse((V3270Accelerator *) accelerator,keyname);

	} else {

		g_autofree gchar *kn = NULL;
		if(id < 13)
			kn = g_strdup_printf("F%u",id);
		else
			kn = g_strdup_printf("<shift>F%u",id-12);

		debug("********* %d - %s",id,kn);

		v3270_accelerator_parse((V3270Accelerator *) accelerator,kn);
	}

	accelerator->parent.activate = G_CALLBACK(fire_pfkey_action);

	return accelerator;
 }

 GSList	* v3270_accelerator_map_load_default(GSList * accelerators)
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

					v3270_accelerator_parse(accelerator,keys[key]);

					accelerators = g_slist_prepend(accelerators,accelerator);

				}

				g_strfreev(keys);

			}
			else
			{
				V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

				accelerator->type = V3270_ACCELERATOR_TYPE_LIB3270_ACTION;
				accelerator->arg  = (gconstpointer) &actions[ix];
				accelerator->activate = G_CALLBACK(fire_lib3270_action);
				accelerators = g_slist_prepend(accelerators,accelerator);

			}

		}

	}

	// Create accelerators for lib3270 toggles.
	{
		const LIB3270_TOGGLE * toggles = lib3270_get_toggles();

		for(ix = 0; toggles[ix].name; ix++)
		{
			V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

			accelerator->type = V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE;
			accelerator->arg = (gconstpointer) &toggles[ix];
			accelerator->activate = G_CALLBACK(fire_lib3270_toggle);

			v3270_accelerator_parse(accelerator,toggles[ix].key);

			accelerators = g_slist_prepend(accelerators,accelerator);

		}

	}

	// Create accelerators for internal actions.
	{
		const V3270_ACTION * actions = v3270_get_actions();

        for(ix = 0 ; actions[ix].name; ix++)
		{
			if(actions[ix].keys && *actions[ix].keys)
			{
				size_t key;

				gchar ** keys = g_strsplit(actions[ix].keys,",",-1);

				for(key = 0; keys[key]; key++)
				{
					V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

					accelerator->type = V3270_ACCELERATOR_TYPE_INTERNAL;
					accelerator->arg = (gconstpointer) &actions[ix];
					accelerator->activate = G_CALLBACK(actions[ix].activate);

					v3270_accelerator_parse(accelerator,keys[key]);

					accelerators = g_slist_prepend(accelerators,accelerator);

				}

				g_strfreev(keys);

			}
			else
			{
				V3270Accelerator * accelerator = g_new0(V3270Accelerator,1);

				accelerator->type = V3270_ACCELERATOR_TYPE_INTERNAL;
				accelerator->arg = (gconstpointer) &actions[ix];
				accelerator->activate = G_CALLBACK(actions[ix].activate);

				accelerators = g_slist_prepend(accelerators,accelerator);

			}
		}

	}

	// Create PF-Key accelerators
	{
		unsigned short key;
		// PF1 - PF24
		for(key = 1; key < 25; key++) {

			accelerators = g_slist_prepend(accelerators,v3270_pfkey_accelerator_new(key,NULL));

			if(key == 7) {
				accelerators = g_slist_prepend(accelerators,v3270_pfkey_accelerator_new(key,"Page_Up"));
			} else if(key == 8) {
				accelerators = g_slist_prepend(accelerators,v3270_pfkey_accelerator_new(key,"Page_Down"));
			}

		}

	}
	/*
	{
		static const struct
		{
			guint 				  key;
			GdkModifierType		  mods;
			const gchar			* name;
			const gchar			* description;
			unsigned short		  pfkey;
		} accels[] =
		{
			{
				.key = GDK_Page_Up,
				.name = "page-up",
				.description = N_( "Previous page" ),
				.pfkey = 7
			},
			{
				.key = GDK_Page_Down,
				.name = "page-down",
				.description = N_( "Next page" ),
				.pfkey = 8
			}
		};

        for(ix = 0 ; ix < G_N_ELEMENTS(accels); ix++)
		{
			V3270PFKeyAccelerator * accelerator = g_new0(V3270PFKeyAccelerator,1);

			accelerator->keycode			= accels[ix].pfkey;
			accelerator->name				= accels[ix].name;
			accelerator->description		= accels[ix].description;
			accelerator->parent.type		= V3270_ACCELERATOR_TYPE_PFKEY;
			accelerator->parent.key			= accels[ix].key;
			accelerator->parent.mods		= accels[ix].mods;
			accelerator->parent.arg			= (gconstpointer) accelerator;
			accelerator->parent.activate	= G_CALLBACK(fire_pfkey_action);

			debug("%s Accelerator(%s)=%p pfkey=%d",__FUNCTION__,accelerator->name,accelerator,accelerator->keycode);

			widget->accelerators = g_slist_prepend(widget->accelerators,accelerator);

		}

	}
	*/

	return v3270_accelerator_map_sort(accelerators);

 }

