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

#ifndef V3270SETTINGS_H_INCLUDED

 #define V3270SETTINGS_H_INCLUDED 1

#ifdef _WIN32
	#include <windows.h>
#endif // _WIN32

 #include <gtk/gtk.h>

 G_BEGIN_DECLS

/*--[ Tools ]----------------------------------------------------------------------------------------*/

 /// @brief Reads the terminal settings from the group group_name in key_file.
 LIB3270_EXPORT gboolean v3270_load_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name);

 /// @brief This function adds the terminal settings from widget to key_file.
 LIB3270_EXPORT void v3270_to_key_file(GtkWidget *widget, GKeyFile *key_file, const gchar *group_name);

#ifdef _WIN32

 LIB3270_EXPORT gboolean v3270_load_registry(GtkWidget *widget, HKEY hKey, const gchar *group_name);

 LIB3270_EXPORT void v3270_to_registry(GtkWidget *widget, HKEY hKey, const gchar *group_name);

#endif // _WIN32

/*--[ V3270 Settings Widget ]------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270_SETTINGS     				(V3270Settings_get_type())
 #define GTK_V3270_SETTINGS(obj)	    			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_SETTINGS, V3270Settings))
 #define GTK_V3270_SETTINGS_CLASS(klass)	    	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_SETTINGS, V3270SettingsClass))
 #define GTK_IS_V3270_SETTINGS(obj)			        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_SETTINGS))
 #define GTK_IS_V3270_SETTINGS_CLASS(klass)	        (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_SETTINGS))
 #define GTK_V3270_SETTINGS_GET_CLASS(obj)		    (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_SETTINGS, V3270SettingsClass))

 typedef struct _V3270Settings {
  	GtkGrid       parent;
    GtkWidget   * terminal;
    const gchar	* label;		///< @brief Label for settings dialog.
    const gchar * title;		///< @brief Title for settings dialog.
    const gchar * tooltip;		///< @brief Tooltip for settings dialog.
  	gboolean	  valid;		///< @brief True if the settings can be applyed.
 } V3270Settings;

 typedef struct _V3270SettingsClass	{
 	GtkGridClass parent_class;

 	void (*load)(GtkWidget *widget, GtkWidget *terminal);			///< @brief Method to load the properties from terminal widget
 	void (*apply)(GtkWidget *widget, GtkWidget *terminal);			///< @brief Method for GTK_RESPONSE_APPLY
 	void (*revert)(GtkWidget *widget, GtkWidget *terminal);			///< @brief Method for GTK_RESPONSE_CANCEL
 	void (*update_message)(GtkWidget *widget, GtkWidget *terminal);	///< @brief Lib3270 message has changed.
	void (*validity)(GtkWidget *, gboolean);
 } V3270SettingsClass;

 LIB3270_EXPORT GType V3270Settings_get_type(void);

 LIB3270_EXPORT void v3270_settings_set_terminal_widget(GtkWidget *widget, GtkWidget *terminal);
 LIB3270_EXPORT GtkWidget * v3270_settings_get_terminal_widget(GtkWidget *widget);

 LIB3270_EXPORT void v3270_settings_apply(GtkWidget *widget);
 LIB3270_EXPORT void v3270_settings_revert(GtkWidget *widget);

 LIB3270_EXPORT void v3270_settings_set_valid(GtkWidget *widget, gboolean is_valid);
 LIB3270_EXPORT gboolean v3270_settings_get_valid(GtkWidget *widget);

 LIB3270_EXPORT const gchar * v3270_settings_get_title(GtkWidget *widget);
 LIB3270_EXPORT const gchar * v3270_settings_get_label(GtkWidget *widget);
 LIB3270_EXPORT const gchar * v3270_settings_get_tooltip(GtkWidget *widget);

 /// @brief Popup a settings dialog for the terminal.
 LIB3270_EXPORT void v3270_settings_popup_dialog(GtkWidget *settings, GtkWidget *terminal, gboolean modal);

 /// @brief Get an edit dialog for the settings widget.
 LIB3270_EXPORT GtkWidget * v3270_settings_get_edit_dialog(GtkWidget *settings, GtkWidget *terminal, gboolean modal);

/*--[ V3270 Settings Dialog ]------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270_SETTINGS_DIALOG				(V3270SettingsDialog_get_type())
 #define GTK_V3270_SETTINGS_DIALOG(obj)	   			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_SETTINGS_DIALOG, V3270SettingsDialog))
 #define GTK_V3270_SETTINGS_DIALOG_CLASS(klass)	    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_SETTINGS_DIALOG, V3270SettingsDialogClass))
 #define GTK_IS_V3270_SETTINGS_DIALOG(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_SETTINGS_DIALOG))
 #define GTK_IS_V3270_SETTINGS_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_SETTINGS_DIALOG))
 #define GTK_V3270_SETTINGS_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_SETTINGS_DIALOG, V3270SettingsDialogClass))

 typedef struct _V3270SettingsDialog V3270SettingsDialog;
 typedef struct _V3270SettingsDialogClass V3270SettingsDialogClass;

 LIB3270_EXPORT GType V3270SettingsDialog_get_type(void);

 LIB3270_EXPORT GtkWidget * v3270_settings_dialog_new();
 LIB3270_EXPORT void v3270_settings_dialog_set_terminal_widget(GtkWidget *widget, GtkWidget *terminal);
 LIB3270_EXPORT void v3270_settings_dialog_set_has_subtitle(GtkWidget *widget, gboolean has_subtitle);

 /// @brief Process GtkDialog's "response" signal.
 LIB3270_EXPORT void v3270_setttings_dialog_response(GtkDialog *dialog, gint response_id, GtkWidget *terminal);

 LIB3270_EXPORT void v3270_settings_dialog_apply(GtkWidget *widget);
 LIB3270_EXPORT void v3270_settings_dialog_revert(GtkWidget *widget);

 LIB3270_EXPORT GtkWidget		* v3270_font_chooser_widget_new() G_GNUC_DEPRECATED;
 LIB3270_EXPORT GtkWidget		* v3270_host_select_new() G_GNUC_DEPRECATED;

 /// @brief Create accelerator settings widget.
 LIB3270_EXPORT GtkWidget		* v3270_accelerator_settings_new();

 /// @brief Create host settings widget.
 LIB3270_EXPORT GtkWidget		* v3270_host_settings_new();

 /// @brief Create font settings widget.
 LIB3270_EXPORT GtkWidget		* v3270_font_settings_new();


 G_END_DECLS

#endif // V3270SETTINGS_H_INCLUDED
