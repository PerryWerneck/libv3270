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
  * @brief Implement GAction who enables/disable based on a v3270 boolean property.
  *
  * Reference:
  *
  * <https://github.com/GNOME/glib/blob/master/gio/gpropertyaction.c>
  *
  */

 #include <internals.h>
 #include <stdlib.h>
 #include <v3270.h>
 #include <v3270/actions.h>
 #include <lib3270/properties.h>

 typedef struct _V3270ConditionalAction {

	V3270SimpleAction parent;

	GParamSpec *pspec;

 } V3270ConditionalAction;

 typedef struct _V3270CopyActionClass {

	V3270SimpleActionClass parent_class;

 } V3270ConditionalActionClass;

 #define V3270_TYPE_CONDITIONAL_ACTION				(V3270ConditionalAction_get_type())
 #define V3270_CONDITIONAL_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_CONDITIONAL_ACTION, V3270ConditionalAction))
 #define V3270_CONDITIONAL_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_CONDITIONAL_ACTION, V3270ConditionalActionClass))
 #define V3270_IS_CONDITIONAL_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_CONDITIONAL_ACTION))
 #define V3270_IS_CONDITIONAL_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_CONDITIONAL_ACTION))
 #define V3270_CONDITIONAL_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_CONDITIONAL_ACTION, V3270ConditionalActionClass))

 static gboolean	get_enabled(GAction *action, GtkWidget *terminal);
 static void		change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(V3270ConditionalAction, V3270ConditionalAction, V3270_TYPE_SIMPLE_ACTION);

 void V3270ConditionalAction_class_init(V3270ConditionalActionClass *klass) {

	klass->parent_class.parent_class.change_widget	= change_widget;
	klass->parent_class.parent_class.get_enabled	= get_enabled;

 }

 static void V3270ConditionalAction_init(V3270ConditionalAction G_GNUC_UNUSED(*action)) {
 }

 static void on_notify(GtkWidget G_GNUC_UNUSED(*terminal), GParamSpec G_GNUC_UNUSED(*pspec), GAction *action) {
 	debug("%s: Enabled of action %s has changed",__FUNCTION__, g_action_get_name(G_ACTION(action)));
 	v3270_action_notify_enabled(action);
 }

 V3270SimpleAction * v3270_conditional_action_new(GtkWidget *widget, const gchar *property_name) {

	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(widget), property_name);

	if(!pspec) {

		g_warning(
			"Can't find property '%s::%s'",
			G_OBJECT_TYPE_NAME(G_OBJECT(widget)),
			property_name
		);

		return NULL;

	}

	debug("%s: pspec(%s)=%p",__FUNCTION__,property_name,pspec);

 	if(~pspec->flags & G_PARAM_READABLE || pspec->flags & G_PARAM_CONSTRUCT_ONLY) {

		g_warning(
			"Property '%s::%s' must be readable and not construct-only",
			G_OBJECT_TYPE_NAME(G_OBJECT(widget)),
			property_name
		);

		return NULL;
    }

 	V3270ConditionalAction * action = (V3270ConditionalAction *) g_object_new(V3270_TYPE_CONDITIONAL_ACTION, NULL);

 	action->parent.name = g_param_spec_get_name(pspec);
 	action->pspec = pspec;

	const LIB3270_PROPERTY * lProperty = lib3270_property_get_by_name(pspec->name);
	if(lProperty) {
		action->parent.label	= lib3270_property_get_label(lProperty);
		action->parent.tooltip 	= lib3270_property_get_summary(lProperty);
//		action->group.id		= lProperty->group;
	}

 	if(!action->parent.tooltip)
		action->parent.tooltip = g_param_spec_get_blurb(pspec);

 	v3270_action_set_terminal_widget(G_ACTION(action), widget);

	return V3270_SIMPLE_ACTION(action);
 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	V3270ConditionalAction * action = V3270_CONDITIONAL_ACTION(object);
	g_autofree gchar * signal_name = g_strconcat("notify::", action->pspec->name,NULL);

	if(from) {
		gulong handler = g_signal_handler_find(
												from,
												G_SIGNAL_MATCH_FUNC|G_SIGNAL_MATCH_DATA,
												0,
												0,
												NULL,
												G_CALLBACK(on_notify),
												action
										);

		if(handler)
			g_signal_handler_disconnect(from, handler);

	}

	V3270_ACTION_CLASS(V3270ConditionalAction_parent_class)->change_widget(object,from,to);

	if(to) {
		g_signal_connect(G_OBJECT(to),signal_name,G_CALLBACK(on_notify),action);
	}

 	v3270_action_notify_enabled(G_ACTION(object));

 }

 gboolean get_enabled(GAction *object, GtkWidget *terminal) {

	gboolean enabled = V3270_ACTION_CLASS(V3270ConditionalAction_parent_class)->get_enabled(object,terminal);

	if(enabled && terminal) {

		// The action is enabled, check property to confirm.
		V3270ConditionalAction * action = V3270_CONDITIONAL_ACTION(object);

		GValue value = G_VALUE_INIT;
		g_value_init(&value, action->pspec->value_type);
		g_object_get_property(G_OBJECT(terminal), action->pspec->name, &value);

		switch(action->pspec->value_type) {
		case G_TYPE_UINT:
			enabled = g_value_get_uint(&value) != 0;
			break;

		case G_TYPE_BOOLEAN:
			enabled = g_value_get_boolean(&value);
			break;

		default:
			enabled = FALSE;
		}

		g_value_unset (&value);

	}

	return enabled;

 }
