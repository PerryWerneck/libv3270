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

#ifndef V3270_SELECTION_H_INCLUDED

    #define V3270_SELECTION_H_INCLUDED 1

    #include <gtk/gtk.h>

    G_BEGIN_DECLS

	// Clipboard
	typedef enum _v3270_copy_mode
	{

		V3270_COPY_DEFAULT,			///< @brief Detect format based on lib3270's smart past toggle.
		V3270_COPY_FORMATTED,		///< @brief Copy formatted data; keep attributes.
		V3270_COPY_TEXT,			///< @brief Single text format, ignore attributes.
		V3270_COPY_TABLE,			///< @brief Copy as table.
		V3270_COPY_APPEND			///< @brief Append to current contents keeping the stored format.

	} V3270_COPY_MODE;

	LIB3270_EXPORT void				  v3270_clipboard_set(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut);
	LIB3270_EXPORT void				  v3270_clipboard_get_from_url(GtkWidget *widget, const gchar *url);

	/// @brief Check if the widget has saved text.
	LIB3270_EXPORT gboolean			  v3270_has_copy(GtkWidget *widget);

	LIB3270_EXPORT void 			  v3270_paste(GtkWidget *widget) G_GNUC_DEPRECATED;
	LIB3270_EXPORT void 			  v3270_paste_text(GtkWidget *widget) G_GNUC_DEPRECATED;
	LIB3270_EXPORT void				  v3270_paste_from_file(GtkWidget *widget) G_GNUC_DEPRECATED;
	LIB3270_EXPORT void				  v3270_copy_selection(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut) G_GNUC_DEPRECATED;
	LIB3270_EXPORT void				  v3270_append_selection(GtkWidget *widget, gboolean cut) G_GNUC_DEPRECATED;

	// Selections

	LIB3270_EXPORT gchar			* v3270_get_selected(GtkWidget *widget, gboolean cut);
	LIB3270_EXPORT gchar			* v3270_get_copy(GtkWidget *widget);

	LIB3270_EXPORT gchar			* v3270_get_text(GtkWidget *widget,int offset, int len);
	LIB3270_EXPORT gchar			* v3270_get_region(GtkWidget *widget, gint start_pos, gint end_pos, gboolean all);

	LIB3270_EXPORT void				  v3270_set_string(GtkWidget *widget, const gchar *str);

	LIB3270_EXPORT gboolean			  v3270_get_selection_bounds(GtkWidget *widget, gint *start, gint *end);
	LIB3270_EXPORT void				  v3270_unselect(GtkWidget *widget);
	LIB3270_EXPORT void 			  v3270_select_all(GtkWidget *widget);
	LIB3270_EXPORT void	  			  v3270_select_region(GtkWidget *widget, gint start, gint end);

	LIB3270_EXPORT void				  v3270_selection_set_font_family(GtkWidget *widget, const gchar *name);
	LIB3270_EXPORT const gchar		* v3270_selection_get_font_family(GtkWidget *widget);

	LIB3270_EXPORT void				  v3270_selection_set_color_scheme(GtkWidget *widget, const gchar *name);
	LIB3270_EXPORT const gchar		* v3270_selection_get_color_scheme(GtkWidget *widget);

	G_END_DECLS

#endif // V3270_ACTIONS_H_INCLUDED
