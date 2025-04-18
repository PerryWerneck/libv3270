/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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

 #include "private.h"
 #include <errno.h>
 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/actions.h>
 #include <v3270/actions.h>
 #include <lib3270/properties.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 V3270Accelerator * v3270_accelerator_clone(const V3270Accelerator *accel)
 {
 	V3270Accelerator * rc = NULL;

 	switch(accel->type)
 	{
 	case V3270_ACCELERATOR_TYPE_CUSTOM:
		{
			V3270CustomAccelerator * customAccel = g_new0(V3270CustomAccelerator,1);
			*customAccel = *((V3270CustomAccelerator *) accel);
			rc = (V3270Accelerator *) customAccel;
		}
		break;

 	case V3270_ACCELERATOR_TYPE_PFKEY:
 		{
 			V3270PFKeyAccelerator * accell = g_new0(V3270PFKeyAccelerator,1);
			*accell = *((V3270PFKeyAccelerator *) accel);
			rc = (V3270Accelerator *) accell;
 		}
 		break;

	default:
		rc = g_new0(V3270Accelerator,1);
		*rc = *accel;

	}

 	return rc;
 }


 gboolean v3270_accelerator_compare(const V3270Accelerator * accel, const guint keyval, const GdkModifierType mods)
 {
	// It's the same key and mods?
	if(accel->key != keyval || accel->mods != mods)
		return FALSE;

	// The same key and same mods, Found it!
	return TRUE;
 }

 void v3270_accelerator_activate(const V3270Accelerator * acel, GtkWidget *terminal)
 {
 	int rc = ((int (*)(GtkWidget *, gconstpointer)) acel->activate)(terminal, acel->arg);

 	debug("%s(%s)=%d (%s)",__FUNCTION__,v3270_accelerator_get_name(acel),rc,strerror(rc));

 	if(rc)
		gdk_display_beep(gtk_widget_get_display(terminal));

 }

 const V3270Accelerator * v3270_get_accelerator(GtkWidget *widget, guint keyval, GdkModifierType state)
 {
	GSList * ix;

	for(ix = GTK_V3270(widget)->accelerators; ix; ix = g_slist_next(ix))
	{
		if(v3270_accelerator_compare((V3270Accelerator *) ix->data, keyval, state))
			return (V3270Accelerator *) ix->data;
	}

#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(keyval,state);
		debug("%s: Can't find accelerator for %s",__FUNCTION__,keyname);
		debug("Keyval: %d (%s) State: %04x %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
				keyval,
				gdk_keyval_name(keyval),
				state,
				state & GDK_SHIFT_MASK		? " GDK_SHIFT_MASK"		: "",
				state & GDK_LOCK_MASK		? " GDK_LOCK_MASK"		: "",
				state & GDK_CONTROL_MASK	? " GDK_CONTROL_MASK"	: "",
				state & GDK_MOD1_MASK		? " GDK_MOD1_MASK"		: "",
				state & GDK_MOD2_MASK		? " GDK_MOD2_MASK"		: "",
				state & GDK_MOD3_MASK		? " GDK_MOD3_MASK"		: "",
				state & GDK_MOD4_MASK		? " GDK_MOD4_MASK"		: "",
				state & GDK_MOD5_MASK		? " GDK_MOD5_MASK"		: "",
				state & GDK_BUTTON1_MASK	? " GDK_BUTTON1_MASK"	: "",
				state & GDK_BUTTON2_MASK	? " GDK_BUTTON2_MASK"	: "",
				state & GDK_BUTTON3_MASK	? " GDK_BUTTON3_MASK"	: "",
				state & GDK_BUTTON4_MASK	? " GDK_BUTTON4_MASK"	: "",
				state & GDK_BUTTON5_MASK	? " GDK_BUTTON5_MASK"	: "",
				state & GDK_RELEASE_MASK	? " GDK_RELEASE_MASK"	: "",
				state & GDK_MODIFIER_MASK	? " GDK_MODIFIER_MASK"	: ""
			);

	}
#endif // DEBUG
	return NULL;

 }

 const gchar * v3270_accelerator_get_summary(const V3270Accelerator * accel) {

	const gchar * description = NULL;
	const LIB3270_PROPERTY * property = (const LIB3270_PROPERTY *) accel->arg;

	switch(accel->type) {
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:
//		debug("%s","V3270_ACCELERATOR_TYPE_LIB3270_ACTION");
		description = lib3270_property_get_summary(property);
		break;

	case V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE:
//		debug("%s","V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE");
		description = lib3270_property_get_summary(property);
		break;

	case V3270_ACCELERATOR_TYPE_INTERNAL:
//		debug("%s","V3270_ACCELERATOR_TYPE_INTERNAL");
		if(property->summary)
			description = g_dgettext(GETTEXT_PACKAGE,property->summary);
		break;

	}

//	debug("%s=%s",__FUNCTION__,description);

	if(description && *description)
		return description;

	return "";
 }

 const gchar * v3270_accelerator_get_description(const V3270Accelerator * accel) {

	const gchar * description = NULL;
	const LIB3270_PROPERTY * property = (const LIB3270_PROPERTY *) accel->arg;

	switch(accel->type) {
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:
		debug("%s","V3270_ACCELERATOR_TYPE_LIB3270_ACTION");
		description = lib3270_property_get_description(property);
		break;

	case V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE:
		debug("%s","V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE");
		description = lib3270_property_get_description(property);
		break;

	case V3270_ACCELERATOR_TYPE_INTERNAL:
		debug("%s","V3270_ACCELERATOR_TYPE_INTERNAL");
		if(property->description)
			description = g_dgettext(GETTEXT_PACKAGE,property->description);
		else if(property->summary)
			description = g_dgettext(GETTEXT_PACKAGE,property->summary);
		break;

	}

	debug("%s=%s",__FUNCTION__,description);

	if(description && *description)
		return description;

	return "";
 }

 void v3270_accelerator_parse(V3270Accelerator * accel, const char *key)
 {
 	if(!key)
		return;

	guint accelerator_key;
	GdkModifierType accelerator_mods;

    gtk_accelerator_parse(key, &accelerator_key, &accelerator_mods);

	accel->key	= gdk_keyval_to_lower(accelerator_key);
	accel->mods = accelerator_mods & gtk_accelerator_get_default_mod_mask();

	/*
	if(!gtk_accelerator_valid(accel->key,accel->mods))
	{
		g_warning("Accelerator \"%s\" is not valid",key);
	}
	*/

	/*
#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(accel->key,accel->mods);
		debug("%s Name: %s Keyval: %d (%s) State: %04x %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
				__FUNCTION__,
				v3270_accelerator_get_name(accel),
				accel->key,
				gdk_keyval_name(accel->key),
				accel->mods,
				accel->mods & GDK_SHIFT_MASK		? " GDK_SHIFT_MASK"		: "",
				accel->mods & GDK_LOCK_MASK			? " GDK_LOCK_MASK"		: "",
				accel->mods & GDK_CONTROL_MASK		? " GDK_CONTROL_MASK"	: "",
				accel->mods & GDK_MOD1_MASK			? " GDK_MOD1_MASK"		: "",
				accel->mods & GDK_MOD2_MASK			? " GDK_MOD2_MASK"		: "",
				accel->mods & GDK_MOD3_MASK			? " GDK_MOD3_MASK"		: "",
				accel->mods & GDK_MOD4_MASK			? " GDK_MOD4_MASK"		: "",
				accel->mods & GDK_MOD5_MASK			? " GDK_MOD5_MASK"		: "",
				accel->mods & GDK_BUTTON1_MASK		? " GDK_BUTTON1_MASK"	: "",
				accel->mods & GDK_BUTTON2_MASK		? " GDK_BUTTON2_MASK"	: "",
				accel->mods & GDK_BUTTON3_MASK		? " GDK_BUTTON3_MASK"	: "",
				accel->mods & GDK_BUTTON4_MASK		? " GDK_BUTTON4_MASK"	: "",
				accel->mods & GDK_BUTTON5_MASK		? " GDK_BUTTON5_MASK"	: "",
				accel->mods & GDK_RELEASE_MASK		? " GDK_RELEASE_MASK"	: "",
				accel->mods & GDK_MODIFIER_MASK	? " GDK_MODIFIER_MASK"	: ""
			);

	}
#endif // DEBUG
	*/
 }

 const gchar * v3270_accelerator_get_name(const V3270Accelerator * accel)
 {
	switch(accel->type)
	{
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:
	case V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE:
	case V3270_ACCELERATOR_TYPE_INTERNAL:
		return lib3270_property_get_name((const LIB3270_PROPERTY *) accel->arg);

	case V3270_ACCELERATOR_TYPE_PFKEY:
		return gdk_atom_name(((const V3270PFKeyAccelerator *) accel)->name);

	case V3270_ACCELERATOR_TYPE_CUSTOM:
		return ((V3270CustomAccelerator *) accel)->name;

	}

	return NULL;
 }

 gchar * v3270_accelerator_get_label(const V3270Accelerator * accel)
 {
    return gtk_accelerator_get_label(accel->key,accel->mods);
 }

