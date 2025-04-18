/*
 * "Software v3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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

 #include <internals.h>
 #include <v3270.h>
 #include <v3270/actions.h>
 #include <lib3270/actions.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

 static void V3270_action_iface_init(GActionInterface *iface);
 static void V3270Action_class_init(V3270ActionClass *klass);
 static void V3270Action_init(V3270Action *action);

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

 static const gchar			* get_null(GAction *action);
 static const GVariantType	* get_state_type(GAction *action);
 static	const GVariantType	* get_parameter_type(GAction *object);
 static LIB3270_ACTION_GROUP get_action_group(GAction *action);

 static void change_widget(GAction *action, GtkWidget *from, GtkWidget *to);
 static void finalize(GObject *object);

 static gboolean			  get_enabled(GAction *action, GtkWidget *terminal);
 static void 				  activate(GAction *action, GVariant *parameter, GtkWidget *terminal);
 static	GVariant 			* get_state(GAction *action, GtkWidget *terminal);
 static const gchar			* translate(GAction *action, const gchar *text);

 static const gchar			* iface_get_name(GAction *action);
 static	const GVariantType	* iface_get_parameter_type(GAction *action);
 static GVariant			* iface_get_state_hint(GAction *action);
 static	const GVariantType	* iface_get_state_type(GAction *action);
 static	GVariant			* iface_get_state(GAction *action);
 static gboolean			  iface_get_enabled(GAction *action);
 static GVariant			* iface_get_state(GAction *object);
 static void				  iface_change_state(GAction *object, GVariant *value);
 static void				  iface_activate(GAction *object, GVariant *parameter);

 enum {
	PROP_NONE,
	PROP_NAME,
	PROP_PARAMETER_TYPE,
	PROP_ENABLED,
	PROP_STATE_TYPE,
	PROP_STATE,
	PROP_ICON_NAME,
	PROP_LABEL,
	PROP_TOOLTIP
 };

 G_DEFINE_TYPE_WITH_CODE(V3270Action, V3270Action, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_ACTION, V3270_action_iface_init))

 void V3270Action_class_init(V3270ActionClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	debug("%s",__FUNCTION__);

	klass->get_name				= get_null;
	klass->get_icon_name		= get_null;
	klass->get_label			= get_null;
	klass->get_tooltip			= get_null;
	klass->get_action_group		= get_action_group;

	klass->change_widget		= change_widget;
	klass->get_enabled			= get_enabled;
	klass->get_state			= get_state;
	klass->translate			= translate;
	klass->activate				= activate;

	klass->get_state_type		= get_state_type;
	klass->get_parameter_type	= get_parameter_type;

 	object_class->finalize		= finalize;
	object_class->get_property	= get_property;
	object_class->set_property	= set_property;

	// Install properties
	g_object_class_install_property(object_class, PROP_NAME,
		g_param_spec_string (
			"name",
			N_("Action Name"),
			N_("The name used to invoke the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_ICON_NAME,
		g_param_spec_string (
			"icon-name",
			N_("Icon Name"),
			N_("The name of the icon associated with the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_LABEL,
		g_param_spec_string (
			"label",
			N_("The action label"),
			N_("The label for the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_TOOLTIP,
		g_param_spec_string (
			"tooltip",
			N_("The action tooltip"),
			N_("The tooltip for the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_PARAMETER_TYPE,
		g_param_spec_boxed ("parameter-type",
			N_("Parameter Type"),
			N_("The type of GVariant passed to activate()"),
			G_TYPE_VARIANT_TYPE,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_STATE_TYPE,
		g_param_spec_boxed ("state-type",
			N_("State Type"),
			N_("The type of the state kept by the action"),
			G_TYPE_VARIANT_TYPE,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	// Enabled property
	klass->properties.enabled =
			g_param_spec_boolean(
				"enabled",
				N_("Enabled"),
				N_("If the action can be activated"),
				TRUE,
				G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
			);

	g_object_class_install_property(object_class, PROP_ENABLED, klass->properties.enabled);

	// State property
	klass->properties.state =
		g_param_spec_variant(
			"state",
			N_("State"),
			N_("The state the action is in"),
			G_VARIANT_TYPE_ANY,
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
		);

	g_object_class_install_property (object_class, PROP_STATE, klass->properties.state);

 }

 void V3270Action_init(V3270Action *action) {

	action->terminal	= NULL;

 }

 void finalize(GObject *object) {

	V3270Action * action = V3270_ACTION(object);

	if(action->terminal) {
		v3270_action_set_terminal_widget(G_ACTION(object),NULL);
		action->terminal = NULL;
	}

	G_OBJECT_CLASS(V3270Action_parent_class)->finalize(object);

 }

 void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	GAction *action = G_ACTION(object);

//	debug("%s(%d)",__FUNCTION__,prop_id);

	switch (prop_id) {
    case PROP_NAME:
		g_value_set_string(value, g_action_get_name(action));
		break;

	case PROP_ICON_NAME:
		g_value_set_string(value, v3270_action_get_icon_name(action));
		break;

	case PROP_LABEL:
		g_value_set_string(value, v3270_action_get_label(action));
		break;

	case PROP_TOOLTIP:
		g_value_set_string(value, v3270_action_get_tooltip(action));
		break;

	case PROP_PARAMETER_TYPE:
		g_value_set_boxed(value, g_action_get_parameter_type(action));
		break;

	case PROP_ENABLED:
		g_value_set_boolean(value, g_action_get_enabled(action));
		break;

	case PROP_STATE_TYPE:
		g_value_set_boxed(value, g_action_get_state_type(action));
		break;

	case PROP_STATE:
		g_value_take_variant(value, g_action_get_state(action));
		break;

	default:
		g_assert_not_reached ();
	}

 }

 void set_property(GObject G_GNUC_UNUSED(*object), guint G_GNUC_UNUSED(prop_id), const GValue G_GNUC_UNUSED(*value), GParamSpec G_GNUC_UNUSED(*pspec)) {
// 	g_message("Action %s property %s is read-only",g_action_get_name(G_ACTION(object)),pspec->name);
 }

 static gboolean bg_notify_enabled(GObject *action) {
// 	debug("%s(%s,%s,%d)",__FUNCTION__,g_action_get_name(G_ACTION(action)),(g_action_get_enabled(G_ACTION(action)) ? "enabled" : "disabled"),(int) v3270_action_get_group(G_ACTION(action)));
	g_object_notify(action, "enabled");
	return FALSE;
 }

 static gboolean bg_notify_state(GObject *action) {
	g_object_notify(action, "state");
	return FALSE;
 }

 void v3270_action_notify_enabled(GAction *action) {
	g_idle_add((GSourceFunc) bg_notify_enabled, G_OBJECT(action));
 }

 void v3270_action_notify_state(GAction *action) {
 	if(g_action_get_state_type(action))
		g_idle_add((GSourceFunc) bg_notify_state, G_OBJECT(action));
 }

 static void event_listener(H3270 G_GNUC_UNUSED(*hSession), void *object) {
	g_idle_add((GSourceFunc) bg_notify_enabled, G_ACTION(object));
 }

 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

 	if(from != to) {

		V3270Action *action			= V3270_ACTION(object);
		LIB3270_ACTION_GROUP group	= v3270_action_get_group(object);

		if(action->listener) {
			lib3270_unregister_action_group_listener(v3270_action_get_session(object),group,action->listener);
			action->listener = NULL;
		}

		action->terminal = to;

		if(group != LIB3270_ACTION_GROUP_NONE && to) {
			action->listener = lib3270_register_action_group_listener(v3270_action_get_session(object),group,event_listener,object);
		}

		g_idle_add((GSourceFunc) bg_notify_enabled, G_OBJECT(action));

		v3270_action_notify_state(object);

 	}

 }

 void v3270_action_set_terminal_widget(GAction *object, GtkWidget *widget) {

	g_return_if_fail(V3270_IS_ACTION(object));

	if(widget) {
		 g_return_if_fail(GTK_IS_V3270(widget));
	}

 	V3270Action * action = V3270_ACTION(object);

 	if(action->terminal != widget) {
		V3270_ACTION_GET_CLASS(object)->change_widget(object,action->terminal,widget);
		action->terminal = widget;
 	}

 }

 GtkWidget * v3270_action_get_terminal_widget(GAction *object) {
	g_return_val_if_fail(V3270_IS_ACTION(object),NULL);
	return V3270_ACTION(object)->terminal;
 }

 gboolean get_enabled(GAction *object, GtkWidget *terminal) {

 	if(terminal == NULL)
		return FALSE;

	LIB3270_ACTION_GROUP group = v3270_action_get_group(object);
	if(group != LIB3270_ACTION_GROUP_NONE) {
		return lib3270_action_group_get_activatable(v3270_get_session(terminal),group) ? TRUE : FALSE;
	}

 	return TRUE;
 }

 void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget G_GNUC_UNUSED(*terminal)) {
	g_message("Action %s can't be activated",g_action_get_name(action));
 }

 //
 // Action methods.
 //
 H3270 * v3270_action_get_session(GAction *action) {
 	g_return_val_if_fail(V3270_IS_ACTION(action),NULL);
 	return v3270_get_session(V3270_ACTION(action)->terminal);
 }

 GAction * v3270_action_new() {
	return G_ACTION(g_object_new(V3270_TYPE_ACTION, NULL));
 }

 GdkPixbuf * v3270_action_get_pixbuf(GAction *action, GtkIconSize icon_size, GtkIconLookupFlags flags) {

	const gchar * icon_name = v3270_action_get_icon_name(action);

	if(!icon_name)
		return NULL;

	return gtk_icon_theme_load_icon(
					gtk_icon_theme_get_default(),
					icon_name,
					icon_size,
					flags,
					NULL
			);

 }

//
// Default methods.
//
 GVariant * get_state(GAction G_GNUC_UNUSED(*object), GtkWidget *terminal) {
	return g_variant_new_boolean(terminal != NULL);
 }

 const gchar * translate(GAction *action, const gchar *msgid) {

	const gchar * domainname = ((V3270Action *) action)->translation_domain ? ((V3270Action *) action)->translation_domain : G_STRINGIFY(PRODUCT_NAME);

	if (msgid && *msgid)
		return (const gchar*) g_dgettext(domainname, msgid);

	return msgid;

 }


//
// Interface Methods.
//
 void V3270_action_iface_init(GActionInterface *iface) {
	iface->get_name				= iface_get_name;
	iface->get_parameter_type	= iface_get_parameter_type;
	iface->get_state_type		= iface_get_state_type;
	iface->get_state_hint		= iface_get_state_hint;
	iface->get_enabled			= iface_get_enabled;
	iface->get_state			= iface_get_state;
	iface->change_state			= iface_change_state;
	iface->activate				= iface_activate;
 }

 const gchar * iface_get_name(GAction *action) {
 	return V3270_ACTION_GET_CLASS(action)->get_name(action);
 }

 GVariant * iface_get_state(GAction *object) {

 	GVariant * state = NULL;

 	if(g_action_get_state_type(object)) {

		GtkWidget * terminal = V3270_ACTION(object)->terminal;

		if(terminal) {
			state = V3270_ACTION_GET_CLASS(object)->get_state(object,terminal);
		} else {
			state = g_variant_new_boolean(FALSE);
		}

		if(state)
			g_variant_ref(state);

 	}

	return state;

 }


 const GVariantType * iface_get_parameter_type(GAction *object) {
	return V3270_ACTION_GET_CLASS(object)->get_parameter_type(object);
 }

 const GVariantType * iface_get_state_type(GAction *object) {
	return V3270_ACTION_GET_CLASS(object)->get_state_type(object);
 }

 GVariant * iface_get_state_hint(GAction G_GNUC_UNUSED(*object)) {
	return NULL;
 }

 void iface_change_state(GAction G_GNUC_UNUSED(*object), GVariant G_GNUC_UNUSED(*value)) {
 	debug("%s",__FUNCTION__);
 }

 gboolean iface_get_enabled(GAction *object) {

 	V3270Action * action = V3270_ACTION(object);

 	if(action && action->terminal) {

		LIB3270_ACTION_GROUP group = v3270_action_get_group(object);

		if(group != LIB3270_ACTION_GROUP_NONE) {

			if(!lib3270_action_group_get_activatable(v3270_get_session(action->terminal),group))
				return FALSE;

		}

		return V3270_ACTION_GET_CLASS(object)->get_enabled(object,action->terminal);
 	}

	return FALSE;

 }

 void iface_activate(GAction *object, GVariant *parameter) {

 	V3270Action * action = V3270_ACTION(object);

 	if(action && action->terminal) {
		V3270_ACTION_GET_CLASS(object)->activate(object,parameter,action->terminal);
 	}

 }

 LIB3270_ACTION_GROUP get_action_group(GAction G_GNUC_UNUSED(*action)) {
 	return LIB3270_ACTION_GROUP_NONE;
 }

 const gchar * get_null(GAction G_GNUC_UNUSED(*action)) {
	return NULL;
 }

  const GVariantType	* get_state_type(GAction G_GNUC_UNUSED(*object)) {
 	return NULL;
 }

 const GVariantType	* get_parameter_type(GAction G_GNUC_UNUSED(*object)) {
 	return NULL;
 }

 const gchar * v3270_action_translate(GAction *action, const gchar *text) {
	return V3270_ACTION_GET_CLASS(action)->translate(action,text);
 }

 LIB3270_ACTION_GROUP v3270_action_get_group(GAction *action) {
 	return V3270_ACTION_GET_CLASS(action)->get_action_group(action);
 }

 const gchar * v3270_action_get_icon_name(GAction *action) {
 	return V3270_ACTION_GET_CLASS(action)->get_icon_name(action);
 }

 const gchar * v3270_action_get_label(GAction *action) {
 	return v3270_action_translate(action,V3270_ACTION_GET_CLASS(action)->get_label(action));
 }

 const gchar * v3270_action_get_tooltip(GAction *action) {
 	return v3270_action_translate(action,v3270_action_translate(action,V3270_ACTION_GET_CLASS(action)->get_tooltip(action)));
 }
