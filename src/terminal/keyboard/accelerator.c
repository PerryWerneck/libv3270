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

 #include "private.h"
 #include <errno.h>
 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/actions.h>
 #include <v3270/actions.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 V3270Accelerator * v3270_accelerator_copy(const V3270Accelerator *accel)
 {
 	V3270Accelerator * rc = NULL;

 	if(accel->type == V3270_ACCELERATOR_TYPE_CUSTOM)
	{
		V3270CustomAccelerator * customAccel = g_new0(V3270CustomAccelerator,1);
		*customAccel = *((V3270CustomAccelerator *) accel);
		rc = (V3270Accelerator *) customAccel;
	}
	else
	{
		rc = g_new0(V3270Accelerator,1);
		*rc = *accel;
	}

 	return rc;
 }


 gboolean v3270_accelerator_compare(const V3270Accelerator * accel, const guint keyval, const GdkModifierType mods)
 {
 	// Problems:

//	debug("%s: keys: %08x %08x",__FUNCTION__,accel->key,keyval);

	// It's the same key?
	if(accel->key != keyval)
	{
		g_autofree gchar * acckey = gtk_accelerator_name(accel->key,accel->mods);
		g_autofree gchar * qkey = gtk_accelerator_name(keyval,mods);
//		debug("%s: Rejected by key %08x %08x (%s %s)",__FUNCTION__,accel->key,keyval,acckey,qkey);
		return FALSE;
	}

	// The same key and same mods, Found it!
 	if(accel->mods == mods)
		return TRUE;

/*
#ifdef DEBUG
	{
		g_autofree gchar * acckey = gtk_accelerator_name(accel->key,accel->mods);
		g_autofree gchar * qkey = gtk_accelerator_name(keyval,mods);
//		debug("%s: accel=%s (%0u/%08x) query=%s (%u/%08x) xor=%08x and=%08x",__FUNCTION__,acckey,accel->key,accel->mods,qkey,keyval,mods,(accel->mods^mods),(accel->mods & mods));
	}
#endif // DEBUG
*/

	return TRUE;
 }

 void v3270_accelerator_activate(const V3270Accelerator * acel, GtkWidget *terminal)
 {
 	int rc = ((int (*)(GtkWidget *, gconstpointer)) acel->activate)(terminal, acel->arg);

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

 const gchar * v3270_accelerator_get_description(const V3270Accelerator * accel)
 {
	switch(accel->type)
	{
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:

		if( ((LIB3270_ACTION *) accel->arg)->summary )
			return gettext(((LIB3270_ACTION *) accel->arg)->summary);

		if( ((LIB3270_ACTION *) accel->arg)->description )
			return gettext(((LIB3270_ACTION *) accel->arg)->description);

		break;

	case V3270_ACCELERATOR_TYPE_INTERNAL:

		if( ((V3270_ACTION *) accel->arg)->summary )
			return gettext(((V3270_ACTION *) accel->arg)->summary);

		if( ((V3270_ACTION *) accel->arg)->description )
			return gettext(((V3270_ACTION *) accel->arg)->description);

		break;

	case V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE:

		if(((LIB3270_TOGGLE *) accel->arg)->summary)
			return gettext(((LIB3270_TOGGLE *) accel->arg)->summary);

		if(((LIB3270_TOGGLE *) accel->arg)->description)
			return gettext(((LIB3270_TOGGLE *) accel->arg)->description);

		break;

	case V3270_ACCELERATOR_TYPE_PFKEY:

		if( ((V3270PFKeyAccelerator *)accel)->name )
			return ((V3270PFKeyAccelerator *)accel)->name;

		break;

	}

	return v3270_accelerator_get_name(accel);
 }

 const gchar * v3270_accelerator_get_name(const V3270Accelerator * accel)
 {
	switch(accel->type)
	{
	case V3270_ACCELERATOR_TYPE_LIB3270_ACTION:
		return ((LIB3270_ACTION *) accel->arg)->name;

	case V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE:
		return ((LIB3270_TOGGLE *) accel->arg)->name;

	case V3270_ACCELERATOR_TYPE_INTERNAL:
		return ((V3270_ACTION *) accel->arg)->name;

	case V3270_ACCELERATOR_TYPE_CUSTOM:
		return ((V3270CustomAccelerator *) accel)->name;

	}

	return NULL;
 }

 gchar * v3270_accelerator_get_label(const V3270Accelerator * accel)
 {
    return gtk_accelerator_get_label(accel->key,accel->mods);
 }

