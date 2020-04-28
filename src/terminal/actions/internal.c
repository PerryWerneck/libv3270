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
 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/actions.h>
 #include <gdk/gdkkeysyms-compat.h>
 #include <v3270/actions.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

 #ifndef GDK_ALT_MASK
	#define GDK_ALT_MASK GDK_MOD1_MASK
 #endif

 #define LIB3270_TYPE_V3270_INTERNAL_ACTION		(V270InternalAction_get_type())
 #define V3270_INTERNAL_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_CAST ((inst), LIB3270_TYPE_V3270_INTERNAL_ACTION, V270InternalAction))

 #define GET_DESCRIPTOR(obj)  ((const V3270_ACTION *) V3270_INTERNAL_ACTION(obj)->definition)

 typedef struct _V270InternalActionClass {
 	V3270ActionClass parent_class;
 } V270InternalActionClass;

 typedef struct _V270InternalAction {
 	V3270Action			  parent;
 	const V3270_ACTION	* definition;
 } V270InternalAction;

 static void V270InternalAction_class_init(V270InternalActionClass *klass);
 static void V270InternalAction_init(V270InternalAction *action);

 G_DEFINE_TYPE(V270InternalAction, V270InternalAction, V3270_TYPE_ACTION);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static const gchar * get_icon_name(GAction *action) {
	return GET_DESCRIPTOR(action)->icon;
 }

 static const gchar * get_label(GAction *action) {
	return GET_DESCRIPTOR(action)->label;
 }

 static const gchar * get_tooltip(GAction *action) {
	return GET_DESCRIPTOR(action)->summary;
 }

 static const gchar * get_name(GAction *action) {
	return GET_DESCRIPTOR(action)->name;
 }

 static void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

 	debug("Activating action \"%s\"",g_action_get_name(action));

 	const V3270_ACTION *descriptor = GET_DESCRIPTOR(action);

 	descriptor->activate(terminal,descriptor);

 }

 static LIB3270_ACTION_GROUP get_action_group(GAction *action) {
	return GET_DESCRIPTOR(action)->group;
 }

 static void V270InternalAction_class_init(V270InternalActionClass *klass) {

 	klass->parent_class.get_name			= get_name;
 	klass->parent_class.get_icon_name		= get_icon_name;
 	klass->parent_class.get_label 			= get_label;
 	klass->parent_class.get_tooltip			= get_tooltip;
 	klass->parent_class.activate			= activate;
 	klass->parent_class.get_action_group	= get_action_group;

 }

 static void V270InternalAction_init(V270InternalAction G_GNUC_UNUSED(*action)) {
 }

 void g_action_map_add_v3270_actions(GActionMap *action_map) {

	const V3270_ACTION * actions = v3270_get_actions();
	size_t ix;

	for(ix = 0; actions[ix].name; ix++) {

		V270InternalAction * action = V3270_INTERNAL_ACTION(g_object_new(LIB3270_TYPE_V3270_INTERNAL_ACTION, NULL));

		action->definition = &actions[ix];
		action->parent.translation_domain = GETTEXT_PACKAGE;

		if(!g_action_get_name(G_ACTION(action))) {
			g_warning("Action \"%s\" is invalid",actions[ix].name);
		} else {
			g_action_map_add_action(action_map,G_ACTION(action));
		}

	}

 }

