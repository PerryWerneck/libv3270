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

 /**
  * @brief Implement GAction "wrapper" for lib3270's actions.
  *
  */

 #include <internals.h>
 #include <lib3270/actions.h>
 #include <v3270.h>
 #include <v3270/actions.h>
 #include "private.h"

 static void Lib3270Action_class_init(Lib3270ActionClass *klass);
 static void Lib3270Action_init(Lib3270Action *action);

 #define LIB3270_ACTION_GET_DESCRIPTOR(obj) ((const LIB3270_ACTION *) LIB3270_ACTION(obj)->definition)

 G_DEFINE_TYPE(Lib3270Action, Lib3270Action, V3270_TYPE_ACTION);

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {

	gboolean enabled = V3270_ACTION_CLASS(Lib3270Action_parent_class)->get_enabled(action,terminal);

 	if(enabled && terminal) {

		H3270 * hSession = v3270_get_session(terminal);
		if(hSession)
			return LIB3270_ACTION_GET_DESCRIPTOR(action)->activatable(hSession) > 0 ? TRUE : FALSE;

 	}

	return FALSE;

 }

 static void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

 	int rc = LIB3270_ACTION_GET_DESCRIPTOR(action)->activate(v3270_get_session(terminal));

 	if(rc) {
		g_message("Can't activate action \"%s\": %s",g_action_get_name(action),strerror(rc));
		gdk_display_beep(gtk_widget_get_display(terminal));
 	}

 }

 static void dispose(GObject *object) {

//	Lib3270Action *action = LIB3270_ACTION(object);


	G_OBJECT_CLASS(Lib3270Action_parent_class)->dispose(object);
 }

 static const gchar * get_name(GAction *action) {
 	return LIB3270_ACTION_GET_DESCRIPTOR(action)->name;
 }

 static const gchar * get_icon_name(GAction *action) {
 	return LIB3270_ACTION_GET_DESCRIPTOR(action)->icon;
 }

 static const gchar * get_label(GAction *action) {
 	return LIB3270_ACTION_GET_DESCRIPTOR(action)->label;
 }

 static const gchar * get_tooltip(GAction *action) {
 	return LIB3270_ACTION_GET_DESCRIPTOR(action)->summary;
 }

 static LIB3270_ACTION_GROUP get_action_group(GAction *action) {
 	return LIB3270_ACTION_GET_DESCRIPTOR(action)->group;
 }

 void Lib3270Action_class_init(Lib3270ActionClass *klass) {

 	klass->parent_class.get_name			= get_name;
	klass->parent_class.get_icon_name		= get_icon_name;
	klass->parent_class.get_label			= get_label;
	klass->parent_class.get_tooltip			= get_tooltip;
	klass->parent_class.get_action_group	= get_action_group;
	klass->parent_class.get_enabled			= get_enabled;
	klass->parent_class.activate 			= activate;

	G_OBJECT_CLASS(klass)->dispose = dispose;

 }

 void Lib3270Action_init(Lib3270Action *action) {
	action->parent.translation_domain = lib3270_get_translation_domain();
 }

 GAction * g_action_new_from_lib3270(const LIB3270_ACTION * definition) {

 	Lib3270Action * action = (Lib3270Action *) g_object_new(LIB3270_TYPE_ACTION, NULL);

	// Setup hooks.
	action->definition = definition;

 	return G_ACTION(action);
 }

 void g_action_map_add_lib3270_actions(GActionMap *action_map) {

	size_t ix;

	const LIB3270_ACTION * actions = lib3270_get_actions();
	for(ix = 0; actions[ix].name; ix++) {

		GAction *action = g_action_new_from_lib3270(&actions[ix]);

		if(!g_action_get_name(action)) {
			g_warning("Action \"%s\" is invalid (no name)",actions[ix].name);
		} else {
			g_action_map_add_action(action_map,action);
		}


	}

 }
