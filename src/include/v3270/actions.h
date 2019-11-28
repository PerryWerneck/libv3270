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

    G_BEGIN_DECLS

    typedef struct _V3270Accelerator V3270Accelerator;

	typedef enum _v3270_action_flag
	{
		V3270_ACTION_FLAG_DEFAULT	= 0x00000000,
		V3270_ACTION_FLAG_CUT		= 0x10000000,
	} V3270_ACTION_FLAGS;

	typedef struct _v3270_action
	{
		LIB3270_PROPERTY_HEAD

		V3270_ACTION_FLAGS	  flags;	///< @brief (The flags for activation.

		guint           	  key;
		GdkModifierType 	  mods;

		int (*activate)(const struct _v3270_action *action, GtkWidget *widget);

	} V3270_ACTION;


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
	LIB3270_EXPORT void			v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action) G_GNUC_DEPRECATED;

	//
	// Keyboard accelerators
	//
	LIB3270_EXPORT void						v3270_backtab(GtkWidget *widget);

	/// @brief Reset accelerator map to defaults.
	LIB3270_EXPORT void						  v3270_accelerator_map_reset(GtkWidget *widget);
	LIB3270_EXPORT void						  v3270_accelerator_map_foreach(GtkWidget *widget,void (*call)(const V3270Accelerator * accel, const char *keys, gpointer ptr), gpointer ptr);
	LIB3270_EXPORT void						  v3270_accelerator_map_to_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name);
	LIB3270_EXPORT V3270Accelerator *		  v3270_accelerator_map_add_entry(GtkWidget *widget, const gchar *name, guint accel_key, GdkModifierType accel_mods, GCallback callback, gpointer data);

	LIB3270_EXPORT const V3270Accelerator	* v3270_get_accelerator(GtkWidget *widget, guint keyval, GdkModifierType state);
	LIB3270_EXPORT void						  v3270_accelerator_activate(const V3270Accelerator * accel, GtkWidget *terminal);
	LIB3270_EXPORT gboolean					  v3270_accelerator_compare(const V3270Accelerator * accel, const guint keyval, const GdkModifierType mods);
	LIB3270_EXPORT const gchar				* v3270_accelerator_get_name(const V3270Accelerator * accel);
	LIB3270_EXPORT const gchar				* v3270_accelerator_get_description(const V3270Accelerator * accel);

	/// @brief Converts the accelerator into a string which can be used to represent the accelerator to the user.
	/// @return A newly-allocated string representing the accelerator.
	LIB3270_EXPORT gchar 					* v3270_accelerator_get_label(const V3270Accelerator * accel);

	G_END_DECLS

#endif // V3270_ACTIONS_H_INCLUDED
