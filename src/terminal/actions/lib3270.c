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

 #define LIB3270_TYPE_ACTION		(Lib3270Action_get_type())
 #define LIB3270_ACTION(inst)		(G_TYPE_CHECK_INSTANCE_CAST ((inst), LIB3270_TYPE_ACTION, Lib3270Action))
 #define LIB3270_IS_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), LIB3270_TYPE_ACTION))

 typedef struct _Lib3270ActionClass {
 	V3270ActionClass parent_class;
 } Lib3270ActionClass;

 typedef struct _Lib3270Action {
 	V3270Action parent;
 } Lib3270Action;

 static void Lib3270Action_class_init(Lib3270ActionClass *klass);
 static void Lib3270Action_init(Lib3270Action *action);

 #define LIB3270_ACTION_GET_DESCRIPTOR(obj) ((LIB3270_ACTION *) ((V3270Action *) obj)->info)

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

	//Lib3270Action *action = LIB3270_ACTION(object);


	G_OBJECT_CLASS(Lib3270Action_parent_class)->dispose(object);
 }

 void Lib3270Action_class_init(Lib3270ActionClass *klass) {

	V3270_ACTION_CLASS(klass)->get_enabled = get_enabled;
	G_OBJECT_CLASS(klass)->dispose = dispose;

 }

 void Lib3270Action_init(Lib3270Action *action) {
	action->parent.activate = activate;
	action->parent.translation_domain = lib3270_get_translation_domain();
 }

 GAction * g_action_new_from_lib3270(const LIB3270_ACTION * definition) {

 	Lib3270Action * action = (Lib3270Action *) g_object_new(LIB3270_TYPE_ACTION, NULL);

	// Setup hooks.
	action->parent.info = (const LIB3270_PROPERTY *) definition;

 	return G_ACTION(action);
 }

 void g_action_map_add_lib3270_actions(GActionMap *action_map) {

	size_t ix;

	const LIB3270_ACTION * actions = lib3270_get_actions();
	for(ix = 0; actions[ix].name; ix++) {

		GAction *action = g_action_new_from_lib3270(&actions[ix]);

		if(!g_action_get_name(action)) {
			g_warning("Action \"%s\" is invalid",actions[ix].name);
		} else {
			g_action_map_add_action(action_map,action);
		}


	}

 }
