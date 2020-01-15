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

#ifndef V3270_ACTIONS_H_INCLUDED

    #define V3270_ACTIONS_H_INCLUDED 1

    #include <gtk/gtk.h>
    #include <lib3270/actions.h>
    #include <lib3270/toggle.h>

    G_BEGIN_DECLS

    typedef struct _V3270Accelerator V3270Accelerator;

	typedef enum _v3270_action_flag
	{
		V3270_ACTION_FLAG_DEFAULT	= 0x00000000,
		V3270_ACTION_FLAG_CUT		= 0x10000000,
	} V3270_ACTION_FLAGS;

	typedef struct _v3270_action V3270_ACTION;

	struct _v3270_action
	{
		LIB3270_PROPERTY_HEAD

		V3270_ACTION_FLAGS	  flags;	///< @brief (The flags for activation).

		guint           	  key;
		GdkModifierType 	  mods;

		int (*activate)(GtkWidget *widget, const V3270_ACTION *action);

	};


	///
	/// @brief Get internal V3270 action table.
	///
	LIB3270_EXPORT const V3270_ACTION * v3270_get_actions();

    //
    // Actions
    //
	LIB3270_EXPORT void			v3270_tab(GtkWidget *widget);

	//
	// Misc
	//
	LIB3270_EXPORT void			v3270_set_scroll_handler(GtkWidget *widget, GdkScrollDirection direction, GCallback callback, gpointer data);

	LIB3270_EXPORT void			v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action) G_GNUC_DEPRECATED;

	//
	// Keyboard accelerators
	//
	LIB3270_EXPORT void						  v3270_backtab(GtkWidget *widget);

	/// @brief Reset accelerator map to defaults.
	LIB3270_EXPORT void						  v3270_accelerator_map_reset(GtkWidget *widget);
	LIB3270_EXPORT void						  v3270_accelerator_map_foreach(GtkWidget *widget,void (*call)(const V3270Accelerator * accel, const char *keys, gpointer ptr), gpointer ptr);
	LIB3270_EXPORT void						  v3270_accelerator_map_to_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name);
	LIB3270_EXPORT gboolean					  v3270_accelerator_map_load_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name);
	LIB3270_EXPORT V3270Accelerator *		  v3270_accelerator_map_add_entry(GtkWidget *widget, const gchar *name, guint accel_key, GdkModifierType accel_mods, GCallback callback, gpointer data);

	LIB3270_EXPORT const V3270Accelerator	* v3270_get_accelerator(GtkWidget *widget, guint keyval, GdkModifierType state);
	LIB3270_EXPORT void						  v3270_accelerator_activate(const V3270Accelerator * accel, GtkWidget *terminal);
	LIB3270_EXPORT gboolean					  v3270_accelerator_compare(const V3270Accelerator * accel, const guint keyval, const GdkModifierType mods);
	LIB3270_EXPORT const gchar				* v3270_accelerator_get_name(const V3270Accelerator * accel);
	LIB3270_EXPORT const gchar				* v3270_accelerator_get_description(const V3270Accelerator * accel);
	LIB3270_EXPORT const gchar				* v3270_accelerator_get_summary(const V3270Accelerator * accel);

	/// @brief Converts the accelerator into a string which can be used to represent the accelerator to the user.
	/// @return A newly-allocated string representing the accelerator.
	LIB3270_EXPORT gchar 					* v3270_accelerator_get_label(const V3270Accelerator * accel);

	LIB3270_EXPORT V3270Accelerator			* v3270_accelerator_clone(const V3270Accelerator *accel);
	LIB3270_EXPORT const V3270Accelerator	* v3270_accelerator_map_lookup_entry(GtkWidget *widget, guint keyval, GdkModifierType state);

	//
	// GAction wrapper for V3270 terminal widget.
	//
	#define V3270_TYPE_ACTION				(V3270Action_get_type())
	#define V3270_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_ACTION, V3270Action))
	#define V3270_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_ACTION, V3270ActionClass))
	#define V3270_IS_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_ACTION))
	#define V3270_IS_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_ACTION))
	#define V3270_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_ACTION, V3270ActionClass))

	typedef struct _V3270Action {

		GObject parent;

		GtkWidget				* terminal;				///> @brief The active terminal widget.
		const LIB3270_PROPERTY	* info;					///> @brief Action info.
		const void 				* listener;				///> @brief Signal listener for the action group.

		const gchar				* translation_domain;	///> @brief The translation domain for the action.

		/// @brief Activation method.
		void (*activate)(GAction *action, GVariant *parameter, GtkWidget *terminal);

	} V3270Action;

	typedef struct _V3270ActionClass {

		GObjectClass parent_class;

		struct {
			GParamSpec * state;
			GParamSpec * enabled;
		} properties;

		void				  (*change_widget)(GAction *action, GtkWidget *from, GtkWidget *to);

		const gchar			* (*translate)(GAction *action, const gchar *text);

		const GVariantType	* (*get_state_type)(GAction *object);
		GVariant			* (*get_state)(GAction *action, GtkWidget *terminal);

		const GVariantType	* (*get_parameter_type)(GAction *object);

		gboolean	  		  (*get_enabled)(GAction *action, GtkWidget *terminal);

		const gchar 		* (*get_name)(GAction *action);
		const gchar 		* (*get_icon_name)(GAction *action);
		const gchar 		* (*get_label)(GAction *action);
		const gchar 		* (*get_tooltip)(GAction *action);

	} V3270ActionClass;

	LIB3270_EXPORT GType		  V3270Action_get_type(void) G_GNUC_CONST;
	LIB3270_EXPORT GAction		* v3270_action_new();

	LIB3270_EXPORT void			  v3270_action_set_terminal_widget(GAction *object, GtkWidget *widget);
	LIB3270_EXPORT GtkWidget	* v3270_action_get_terminal_widget(GAction *object);

	LIB3270_EXPORT void			  v3270_action_notify_state(GAction *action);
	LIB3270_EXPORT void			  v3270_action_notify_enabled(GAction *action);

	LIB3270_EXPORT const gchar	* v3270_action_translate(GAction *action, const gchar *text);

	LIB3270_EXPORT H3270 		* v3270_action_get_session(GAction *action);
	LIB3270_EXPORT const gchar	* v3270_action_get_icon_name(GAction *action);

	LIB3270_EXPORT const gchar	* v3270_action_get_label(GAction *action);
	LIB3270_EXPORT const gchar	* v3270_action_get_tooltip(GAction *action);
	LIB3270_EXPORT GdkPixbuf	* v3270_action_get_pixbuf(GAction *action, GtkIconSize icon_size, GtkIconLookupFlags flags);

	LIB3270_EXPORT GAction		* g_action_new_from_lib3270(const LIB3270_ACTION * definition);
	LIB3270_EXPORT GAction		* g_action_new_from_toggle(const LIB3270_TOGGLE * definition);

	LIB3270_EXPORT GAction		* v3270_pfkey_action_new(void);
	LIB3270_EXPORT GAction		* v3270_pakey_action_new(void);

	LIB3270_EXPORT void			  g_action_map_add_v3270_actions(GActionMap *action_map);
	LIB3270_EXPORT void			  g_action_map_add_lib3270_actions(GActionMap *action_map);
	LIB3270_EXPORT void			  g_action_map_add_lib3270_toggles(GActionMap *action_map);

	//
	// "Simple" action
	//
	#define V3270_TYPE_SIMPLE_ACTION				(V3270SimpleAction_get_type())
	#define V3270_SIMPLE_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_SIMPLE_ACTION, V3270SimpleAction))
	#define V3270_SIMPLE_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_SIMPLE_ACTION, V3270SimpleActionClass))
	#define V3270_IS_SIMPLE_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_SIMPLE_ACTION))
	#define V3270_IS_SIMPLE_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_SIMPLE_ACTION))
	#define V3270_SIMPLE_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_SIMPLE_ACTION, V3270SimpleActionClass))

	typedef struct _V3270SimpleAction {

		V3270Action parent;

		// Fixed data
		const gchar * name;
		const gchar * icon_name;
		const gchar	* label;
		const gchar	* tooltip;

		// Lib3270 Action group
		struct {
			LIB3270_ACTION_GROUP id;
			const void * listener;
		} group;

		/// @brief Activation method.
		void (*activate)(GAction *action, GVariant *parameter, GtkWidget *terminal);

	} V3270SimpleAction;

	typedef struct _V3270SimpleActionClass {

		V3270ActionClass parent_class;

	} V3270SimpleActionClass;

	GType V3270SimpleAction_get_type(void) G_GNUC_CONST;

	/// @brief Create an empty simple action.
	V3270SimpleAction * v3270_simple_action_new();

	/// @brief Create a dialog action.
	V3270SimpleAction * v3270_dialog_action_new(GtkWidget * (*factory)(V3270SimpleAction *, GtkWidget *));

	/// @brief Create an action from property name.
	V3270SimpleAction * v3270_property_action_new(GtkWidget *widget, const gchar *property_name);

	/// @brief Create an action with the "enable" property binded with terminal property.
	V3270SimpleAction * v3270_conditional_action_new(GtkWidget *widget, const gchar *property_name);

	G_END_DECLS

#endif // V3270_ACTIONS_H_INCLUDED
