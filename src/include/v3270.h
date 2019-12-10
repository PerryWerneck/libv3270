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

#ifndef V3270_H_INCLUDED

 #define V3270_H_INCLUDED 1

 #include <glib.h>
 #include <gtk/gtk.h>
 #include <lib3270.h>
 #include <lib3270/popup.h>
 #include <lib3270/filetransfer.h>

#if ! GLIB_CHECK_VERSION(2,44,0)

	LIB3270_EXPORT void v3270_autoptr_cleanup_generic_gfree(void *p);

	#define g_autofree __attribute__((cleanup(v3270_autoptr_cleanup_generic_gfree)))

	static inline void v3270_autoptr_cleanup_GError(GError **error) {
		if(*error) {
			g_error_free(*error);
			*error = NULL;
		}
	}

	static inline void v3270_autoptr_cleanup_GObject(GObject **object) {
		if(*object) {
			g_object_unref(*object);
			*object = NULL;
		}
	}

	static inline void v3270_autoptr_cleanup_GVariant(GVariant **variant) {
		if(*variant) {
			g_variant_unref(*variant);
		}
	}

	#define V3270_AUTOPTR_FUNC_NAME(TypeName) v3270_autoptr_cleanup_##TypeName
	#define g_autoptr(TypeName) TypeName * __attribute__ ((__cleanup__(V3270_AUTOPTR_FUNC_NAME(TypeName))))

#endif // ! GLIB(2,44,0)

 G_BEGIN_DECLS

 #define GTK_TYPE_V3270				(v3270_get_type ())
 #define GTK_V3270(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270, v3270))
 #define GTK_V3270_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270, v3270Class))
 #define GTK_IS_V3270(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270))
 #define GTK_IS_V3270_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270))
 #define GTK_V3270_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270, v3270Class))

#if ! GTK_CHECK_VERSION(3,0,0)
    #define GdkRGBA                                     GdkColor
    #define gdk_cairo_set_source_rgba(cr,cl)            gdk_cairo_set_source_color(cr,cl)
    #define gdk_rgba_parse(a,b)                         gdk_color_parse(b,a)
    #define gdk_rgba_to_string(c)                       gdk_color_to_string(c)
    #define gdk_rgba_equal(a,b)                         gdk_color_equal(a,b)
    #define gdk_rgba_to_string(c)                       gdk_color_to_string(c)
    #define gtk_color_selection_set_current_rgba(w,c)   gtk_color_selection_set_current_color(w,c)
    #define gtk_color_selection_get_current_rgba(w,c)   gtk_color_selection_get_current_color(w,c)
	#define gtk_color_selection_set_previous_rgba(w,c)  gtk_color_selection_set_previous_color(w,c)
	#define gtk_color_selection_set_current_rgba(w,c)   gtk_color_selection_set_current_color(w,c)
#endif // !GTK(3,0,0)


 typedef struct _v3270			v3270;
 typedef struct _v3270Class		v3270Class;

 enum V3270_COLOR
 {
	V3270_COLOR_BACKGROUND,
	V3270_COLOR_BLUE,
	V3270_COLOR_RED,
	V3270_COLOR_PINK,
	V3270_COLOR_GREEN,
	V3270_COLOR_TURQUOISE,
	V3270_COLOR_YELLOW,
	V3270_COLOR_WHITE,
	V3270_COLOR_BLACK,
	V3270_COLOR_DARK_BLUE,
	V3270_COLOR_ORANGE,
	V3270_COLOR_PURPLE,
	V3270_COLOR_DARK_GREEN,
	V3270_COLOR_DARK_TURQUOISE,
	V3270_COLOR_MUSTARD,
	V3270_COLOR_GRAY,

	V3270_COLOR_FIELD,
	V3270_COLOR_FIELD_INTENSIFIED,
	V3270_COLOR_FIELD_PROTECTED,
	V3270_COLOR_FIELD_PROTECTED_INTENSIFIED,

	V3270_COLOR_SELECTED_BG,
	V3270_COLOR_SELECTED_FG,

	V3270_COLOR_CROSS_HAIR,

	// Oia Colors (Must be the last block)
	V3270_COLOR_OIA_BACKGROUND,
	V3270_COLOR_OIA_FOREGROUND,
	V3270_COLOR_OIA_SEPARATOR,
	V3270_COLOR_OIA_STATUS_OK,
	V3270_COLOR_OIA_STATUS_WARNING,
	V3270_COLOR_OIA_STATUS_INVALID,

	V3270_COLOR_COUNT
 };

// #define V3270_COLOR_OIA_STATUS_WARNING V3270_COLOR_OIA_STATUS_OK

 typedef enum _v3270_oia_field
 {
	V3270_OIA_UNDERA,			/**< "A" underlined */
	V3270_OIA_CONNECTION,		/**< solid box if connected, "?" in a box if not */
	V3270_OIA_MESSAGE,			/**< message area */
	V3270_OIA_SSL,				/**< SSL Status */
								/**< Meta indication ("M" or blank) */
	V3270_OIA_ALT,				/**< Alt indication ("A" or blank) */
								/**< Compose indication ("C" or blank) */
								/**< Compose first character */
	V3270_OIA_SHIFT,			/**< Shift Status */
	V3270_OIA_TYPEAHEAD,		/**< Typeahead indication ("T" or blank) */
	V3270_OIA_INSERT,			/**< Insert mode indication (Special symbol/"I" or blank) */
	V3270_OIA_SCRIPT,			/**< Script indication  ("S" or blank) */
	V3270_OIA_LUNAME,			/**< LU Name */
	V3270_OIA_SPINNER,			/**< command timing spinner */
	V3270_OIA_TIMER,			/**< command timing (mmm:ss, or blank) */
	V3270_OIA_CURSOR_POSITION,	/**< cursor position (rrr/ccc or blank) */

//	V3270_OIA_CAPS,				/**< Caps indication ("A" or blank) */

#ifdef HAVE_PRINTER
	V3270_OIA_PRINTER,			/**< Printer indication ("P" or blank) */
#endif // HAVE_PRINTER

	V3270_OIA_FIELD_COUNT

 } V3270_OIA_FIELD;

 #define V3270_OIA_FIELD_INVALID	((V3270_OIA_FIELD) -1)


 #define V3270_COLOR_OIA_SPINNER 	V3270_COLOR_OIA_FOREGROUND
 #define V3270_COLOR_OIA_LUNAME		V3270_COLOR_OIA_FOREGROUND
 #define V3270_COLOR_OIA_INSERT		V3270_COLOR_OIA_FOREGROUND

 #ifndef v3270char
    #define v3270char void
 #endif // v3270_char

 LIB3270_EXPORT GtkWidget		* v3270_new(void);
 LIB3270_EXPORT GType 			  v3270_get_type(void);
 LIB3270_EXPORT void		  	  v3270_reload(GtkWidget * widget);
 LIB3270_EXPORT void			  v3270_set_font_family(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT const gchar		* v3270_get_font_family(GtkWidget *widget);
 LIB3270_EXPORT H3270			* v3270_get_session(GtkWidget *widget);
 LIB3270_EXPORT gboolean		  v3270_is_connected(GtkWidget *widget);
 LIB3270_EXPORT int				  v3270_reconnect(GtkWidget *widget);
 LIB3270_EXPORT void			  v3270_disconnect(GtkWidget *widget);
 LIB3270_EXPORT int				  v3270_set_host_charset(GtkWidget *widget, const gchar *name);

 LIB3270_EXPORT void			  v3270_set_auto_disconnect(GtkWidget *widget, guint minutes);
 LIB3270_EXPORT guint			  v3270_get_auto_disconnect(GtkWidget *widget);

 LIB3270_EXPORT void			  v3270_set_dynamic_font_spacing(GtkWidget *widget, gboolean state);
 LIB3270_EXPORT gboolean		  v3270_get_dynamic_font_spacing(GtkWidget *widget);

 LIB3270_EXPORT void			  v3270_set_lunames(GtkWidget *widget, const gchar *lunames);
 LIB3270_EXPORT gchar 			* v3270_get_lunames(GtkWidget *widget);

 // Clipboard
 typedef enum _v3270_copy_mode
 {

 	V3270_COPY_DEFAULT,			///< @brief Detect format based on lib3270's smart past toggle.
 	V3270_COPY_FORMATTED,		///< @brief Copy formatted data; keep attributes.
	V3270_COPY_TEXT,			///< @brief Single text format, ignore attributes.
	V3270_COPY_TABLE,			///< @brief Copy as table.
	V3270_COPY_APPEND			///< @brief Append to current contents keeping the stored format.

 } V3270_COPY_MODE;

 LIB3270_EXPORT void			  v3270_clipboard_set(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut);
 LIB3270_EXPORT void			  v3270_clipboard_get_from_url(GtkWidget *widget, const gchar *url);

 /// @brief Check if the widget has saved text.
 LIB3270_EXPORT gboolean		  v3270_has_copy(GtkWidget *widget);

 LIB3270_EXPORT void 			  v3270_paste(GtkWidget *widget) G_GNUC_DEPRECATED;
 LIB3270_EXPORT void 			  v3270_paste_text(GtkWidget *widget) G_GNUC_DEPRECATED;
 LIB3270_EXPORT void			  v3270_paste_from_file(GtkWidget *widget) G_GNUC_DEPRECATED;
 LIB3270_EXPORT void			  v3270_copy_selection(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut) G_GNUC_DEPRECATED;
 LIB3270_EXPORT void			  v3270_append_selection(GtkWidget *widget, gboolean cut) G_GNUC_DEPRECATED;

 // Selections

 LIB3270_EXPORT gchar			* v3270_get_selected(GtkWidget *widget, gboolean cut);
 LIB3270_EXPORT gchar			* v3270_get_copy(GtkWidget *widget);

 LIB3270_EXPORT gchar			* v3270_get_text(GtkWidget *widget,int offset, int len);
 LIB3270_EXPORT gchar			* v3270_get_region(GtkWidget *widget, gint start_pos, gint end_pos, gboolean all);

 LIB3270_EXPORT void			  v3270_set_string(GtkWidget *widget, const gchar *str);

 LIB3270_EXPORT gboolean		  v3270_get_selection_bounds(GtkWidget *widget, gint *start, gint *end);
 LIB3270_EXPORT void			  v3270_unselect(GtkWidget *widget);
 LIB3270_EXPORT void 			  v3270_select_all(GtkWidget *widget);
 LIB3270_EXPORT void	  		  v3270_select_region(GtkWidget *widget, gint start, gint end);

 LIB3270_EXPORT void 			  v3270_input_text(GtkWidget *widget, const gchar *text, const gchar *encoding);

 // Colors
 LIB3270_EXPORT void			  v3270_set_colors(GtkWidget *widget, const gchar *);
 LIB3270_EXPORT void 			  v3270_set_color_table(GdkRGBA *table, const gchar *colors);
 LIB3270_EXPORT const GdkRGBA	* v3270_get_color_table(GtkWidget *widget);
 LIB3270_EXPORT void 			  v3270_set_mono_color_table(GdkRGBA *table, const gchar *fg, const gchar *bg);
 LIB3270_EXPORT void			  v3270_set_color(GtkWidget *widget, enum V3270_COLOR id, const GdkRGBA *color);
 LIB3270_EXPORT GdkRGBA 		* v3270_get_color(GtkWidget *widget, enum V3270_COLOR id);

 // Misc
 LIB3270_EXPORT GtkIMContext	* v3270_get_im_context(GtkWidget *widget);
 LIB3270_EXPORT const gchar		* v3270_get_default_font_name();

 LIB3270_EXPORT void			  v3270_popup_message(GtkWidget *widget, LIB3270_NOTIFY type, const gchar *title, const gchar *message, const gchar *text);

 LIB3270_EXPORT const gchar		* v3270_get_session_name(GtkWidget *widget);
 LIB3270_EXPORT void			  v3270_set_session_name(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT gchar			* v3270_get_session_title(GtkWidget *widget);

// LIB3270_EXPORT gchar			* v3270_get_title(GtkWidget *widget);
 LIB3270_EXPORT gchar			* v3270_set_title(GtkWidget *widget, const gchar *title);

 LIB3270_EXPORT int				  v3270_set_script(GtkWidget *widget, const gchar id);
 LIB3270_EXPORT void			  v3270_set_scaled_fonts(GtkWidget *widget, gboolean on);
 LIB3270_EXPORT int				  v3270_set_session_color_type(GtkWidget *widget, unsigned short colortype);

 LIB3270_EXPORT int				  v3270_set_host_type(GtkWidget *widget, LIB3270_HOST_TYPE type);
 LIB3270_EXPORT int				  v3270_set_host_type_by_name(GtkWidget *widget, const char *name);

 LIB3270_EXPORT void			  v3270_set_url(GtkWidget *widget, const gchar *uri);
 LIB3270_EXPORT const gchar		* v3270_get_url(GtkWidget *widget);

 LIB3270_EXPORT const char		* v3270_get_associated_luname(const GtkWidget *widget);

 LIB3270_EXPORT void			  v3270_set_session_host_type(GtkWidget *widget, LIB3270_HOST_TYPE);

 LIB3270_EXPORT	void			  v3270_set_remap_filename(GtkWidget *widget, const gchar *path);
 LIB3270_EXPORT const gchar		* v3270_get_remap_filename(GtkWidget *widget);

 // SSL & Security
 LIB3270_EXPORT const gchar		* v3270_get_ssl_state_icon_name(GtkWidget *widget);
 LIB3270_EXPORT const gchar		* v3270_get_ssl_state_message(GtkWidget *widget);
 LIB3270_EXPORT const gchar		* v3270_get_ssl_state_description(GtkWidget *widget);
 LIB3270_EXPORT GtkWidget 		* v3270_security_dialog_new(GtkWidget *widget);
 LIB3270_EXPORT void			  v3270_popup_security_dialog(GtkWidget *widget);

 // Auxiliary widgets
 LIB3270_EXPORT void			  v3270_select_host(GtkWidget *widget);

 // Print
 LIB3270_EXPORT int				  v3270_print(GtkWidget *widget, GError **error);
 LIB3270_EXPORT int				  v3270_print_all(GtkWidget *widget, GError **error);
 LIB3270_EXPORT int				  v3270_print_selected(GtkWidget *widget, GError **error);
 LIB3270_EXPORT int				  v3270_print_copy(GtkWidget *widget, GError **error);
 LIB3270_EXPORT int				  v3270_print_dialog(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, GError **error);

 // Save
 LIB3270_EXPORT int				  v3270_save(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, const gchar *filename, GError **error);
 LIB3270_EXPORT int				  v3270_save_all(GtkWidget *widget, const gchar *filename, GError **error);
 LIB3270_EXPORT int				  v3270_save_selected(GtkWidget *widget, const gchar *filename, GError **error);
 LIB3270_EXPORT int				  v3270_save_copy(GtkWidget *widget, const gchar *filename, GError **error);

 // Misc
 LIB3270_EXPORT void			  v3270_zoom_best(GtkWidget *widget);
 LIB3270_EXPORT void			  v3270_zoom_in(GtkWidget *widget);
 LIB3270_EXPORT void			  v3270_zoom_out(GtkWidget *widget);

 // Convenience
 LIB3270_EXPORT void			  gtk_entry_set_printf(GtkEntry *entry, const gchar *fmt, ...) G_GNUC_PRINTF(2,3);

 G_END_DECLS

#endif // V3270_H_INCLUDED
