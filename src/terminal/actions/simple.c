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
  * @brief Implement V3270 Simple Action.
  *
  */

 #include "private.h"
 #include <v3270.h>
 #include <v3270/actions.h>

 static void V3270SimpleAction_class_init(V3270SimpleActionClass *klass);
 static void V3270SimpleAction_init(V3270SimpleAction *action);

 G_DEFINE_TYPE(V3270SimpleAction, V3270SimpleAction, V3270_TYPE_ACTION);

 static const gchar * get_icon_name(GAction *action) {
	return V3270_SIMPLE_ACTION(action)->icon_name;
 }

 static const gchar * get_label(GAction *action) {
	return V3270_SIMPLE_ACTION(action)->label;
 }

 static const gchar * get_tooltip(GAction *action) {
	return V3270_SIMPLE_ACTION(action)->tooltip;
 }

 static const gchar * get_name(GAction *action) {
	return V3270_SIMPLE_ACTION(action)->name;
 }

 static void dispose(GObject *object) {

	V3270SimpleAction *action = V3270_SIMPLE_ACTION(object);

	if(action->group.listener) {
		lib3270_unregister_action_group_listener(v3270_action_get_session(G_ACTION(object)),action->group.id,action->group.listener);
		action->group.listener = NULL;
	}

	G_OBJECT_CLASS(V3270SimpleAction_parent_class)->dispose(object);
 }

 static void klass_activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {
 	V3270_SIMPLE_ACTION(action)->activate(action,parameter,terminal);
 }

 static void V3270SimpleAction_class_init(V3270SimpleActionClass *klass) {

 	klass->parent_class.get_name		= get_name;
 	klass->parent_class.get_icon_name	= get_icon_name;
 	klass->parent_class.get_label 		= get_label;
 	klass->parent_class.get_tooltip		= get_tooltip;
 	klass->parent_class.activate		= klass_activate;

	G_OBJECT_CLASS(klass)->dispose = dispose;

 }

 static void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget G_GNUC_UNUSED(*terminal)) {
 	g_warning("Action %s activation method is invalid",g_action_get_name(action));
 }

 static void V3270SimpleAction_init(V3270SimpleAction *action) {

	action->icon_name		= NULL;
	action->label			= _( "No label" );
	action->tooltip			= NULL;
	action->activate		= activate;
	action->group.id		= LIB3270_ACTION_GROUP_NONE;
	action->group.listener	= NULL;

 }

 V3270SimpleAction * v3270_simple_action_new() {
 	return (V3270SimpleAction *) g_object_new(V3270_TYPE_SIMPLE_ACTION, NULL);
 }



