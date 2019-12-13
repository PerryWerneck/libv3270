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

#ifndef V3270_INTERNALS_H_INCLUDED

 #define V3270_INTERNALS_H_INCLUDED 1

 #include <config.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <glib.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <v3270.h>
 #include <v3270/toggle.h>

 G_BEGIN_DECLS

 /* not really I18N-related, but also a string marker macro */
 #define I_(string) g_intern_static_string (string)

/*--[ Structures ]-----------------------------------------------------------------------------------*/

 struct v3270_character
 {
	unsigned char	  chr;		///< @brief Character value.
	unsigned short	  attr;		///< @brief Character attribute.
 };

 /// @brief Head for settings widgets.
 #define ENTRY_FIELD_HEAD \
			gint left; \
			gint top; \
			gint width; \
			gint height; \
			const gchar * label; \
			const gchar * tooltip;

 struct v3270_entry_field
 {
 	ENTRY_FIELD_HEAD
 };

//--[ Globals ]--------------------------------------------------------------------------------------

 /// @brief V3270 Signal list
 enum
 {
 	V3270_SIGNAL_TOGGLE_CHANGED,
 	V3270_SIGNAL_MESSAGE_CHANGED,
 	V3270_SIGNAL_KEYPRESS,
 	V3270_SIGNAL_MODEL_CHANGED,
 	V3270_SIGNAL_POPUP,
 	V3270_SIGNAL_CHANGED,
 	V3270_SIGNAL_MESSAGE,
 	V3270_SIGNAL_FIELD,
 	V3270_SIGNAL_SESSION_CHANGED,

 	//
 	// Clipboard signals.
 	//
 	V3270_SIGNAL_SELECTING,
 	V3270_SIGNAL_CLIPBOARD,
 	V3270_SIGNAL_PASTENEXT,

 	//
 	// Network signals.
 	//
 	V3270_SIGNAL_CONNECTED,
 	V3270_SIGNAL_DISCONNECTED,

 	//
 	// Print session signals.
 	//
 	V3270_SIGNAL_PRINT_SETUP,
 	V3270_SIGNAL_PRINT_APPLY,
 	V3270_SIGNAL_PRINT_DONE,

 	//
 	// Settings signals (Mostly fired by V3270Settings dialogs).
 	//
 	V3270_SIGNAL_SAVE_SETTINGS,		///< @brief Notify main application to save all widget settings.
 	V3270_SIGNAL_UPDATE_CONFIG,		///< @brief Update config signal (deprecated!)

 	V3270_SIGNAL_LAST
 };

 G_GNUC_INTERNAL guint v3270_widget_signal[V3270_SIGNAL_LAST];

/*--[ Constants ]------------------------------------------------------------------------------------*/

 G_GNUC_INTERNAL const gchar	* v3270_default_colors;

/*--[ Internal methods ]-----------------------------------------------------------------------------*/

 G_GNUC_INTERNAL void			  v3270_drag_dest_set(GtkWidget *widget, GCallback callback);

 G_GNUC_INTERNAL GtkWidget		* v3270_box_pack_start(GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding);
 G_GNUC_INTERNAL GtkWidget		* v3270_box_pack_end(GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding);
 G_GNUC_INTERNAL GtkWidget		* v3270_box_pack_frame(GtkWidget *box, GtkWidget *child, const gchar *title, const gchar *tooltip, GtkAlign align, gboolean expand, gboolean fill, guint padding);
 G_GNUC_INTERNAL GtkWidget		* v3270_dialog_create_grid(GtkAlign align);
 G_GNUC_INTERNAL GtkWidget		* v3270_dialog_create_frame(GtkWidget * child, const gchar *title);

 G_GNUC_INTERNAL void			  v3270_dialog_close(GtkDialog *dialog, gpointer user_data);

 /// @brief Emit the "save-settings" signal.
 G_GNUC_INTERNAL void			  v3270_emit_save_settings(GtkWidget *widget);

 /// @brief Emit "has-text" signal.
 G_GNUC_INTERNAL void			  v3270_emit_copy_state(GtkWidget *widget);

 G_GNUC_INTERNAL gchar 			* v3270_convert_to_3270_charset(GtkWidget *widget, const gchar *text, const gchar *encoding, GError **error);

 /// @brief Add current lib3270 selection to a list
 G_GNUC_INTERNAL GList			* g_list_append_lib3270_selection(GList *list, H3270 *hSession, gboolean all);

 G_GNUC_INTERNAL const gchar 	* v3270_translate_cg_to_utf(unsigned char chr);

/*--[ Internal Widgets & Tools ]---------------------------------------------------------------------*/

 enum
 {
 	V3270_ACCELERATOR_TYPE_INTERNAL,			///< @brief Accelerator is internal.
 	V3270_ACCELERATOR_TYPE_LIB3270_ACTION,		///< @brief Accelerator is a lib3270 action.
 	V3270_ACCELERATOR_TYPE_LIB3270_TOGGLE,		///< @brief Accelerator is a lib3270 toggle.
 	V3270_ACCELERATOR_TYPE_PFKEY,				///< @brief Accelerator is a PFKey redirector.
 	V3270_ACCELERATOR_TYPE_CUSTOM,				///< @brief Custom (application based) accelerator.
 };

 struct _V3270Accelerator
 {
 	unsigned short		type;
	guint           	key;
	GdkModifierType 	mods;
	gconstpointer		arg;
	GCallback			activate;
 };

 typedef struct _V3270CustomAccelerator
 {
 	struct _V3270Accelerator parent;
 	const gchar *name;
 } V3270CustomAccelerator;

 typedef struct _V3270PFKeyAccelerator
 {
 	struct _V3270Accelerator parent;
 	const gchar *name;
 	const gchar *description;
 	unsigned short keycode;
 } V3270PFKeyAccelerator;

 typedef enum v3270_toggleable_dialog
 {
	V3270_TOGGLEABLE_DIALOG_PASTE_FAILED,

	V3270_TOGGLEABLE_DIALOG_CUSTOM
 } V3270_TOGGLEABLE_DIALOG;

 G_GNUC_INTERNAL GtkResponseType v3270_popup_toggleable_dialog(GtkWidget *widget, V3270_TOGGLEABLE_DIALOG id, const gchar *title, const gchar *summary, const gchar *body, const gchar *first_button_text, ...) G_GNUC_NULL_TERMINATED;

#if GTK_CHECK_VERSION(3,12,0)
 G_GNUC_INTERNAL GtkHeaderBar	* v3270_dialog_get_header_bar(GtkWidget * widget);
#endif // ! GTK 3.12

 G_GNUC_INTERNAL void			  v3270_grid_attach(GtkGrid *grid, const struct v3270_entry_field * description, GtkWidget *widget);

 // Toggle button widget
 #define GTK_TYPE_V3270_TOGGLE_BUTTON				(V3270ToggleButton_get_type ())
 #define GTK_V3270_TOGGLE_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_TOGGLE_BUTTON, V3270ToggleButton))
 #define GTK_V3270_TOGGLE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_TOGGLE_BUTTON, V3270ToggleButtonClass))
 #define GTK_IS_V3270_TOGGLE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_TOGGLE_BUTTON))
 #define GTK_IS_V3270_TOGGLE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_TOGGLE_BUTTON))
 #define GTK_V3270_TOGGLE_BUTTON_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_TOGGLE_BUTTON, V3270ToggleButtonClass))

 typedef struct _V3270ToggleButton		V3270ToggleButton;
 typedef struct _V3270ToggleButtonClass	V3270ToggleButtonClass;

 G_GNUC_INTERNAL GtkWidget	* v3270_toggle_button_new(LIB3270_TOGGLE_ID toggle);
 G_GNUC_INTERNAL void		  v3270_toggle_button_set_session(GtkWidget *button, H3270 *hSession);

 // Activity list widget.
 #define GTK_TYPE_V3270_FT_ACTIVITY_LIST				(V3270FTActivityList_get_type ())
 #define GTK_V3270_FT_ACTIVITY_LIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_FT_ACTIVITY_LIST, V3270FTActivityList))
 #define GTK_V3270_FT_ACTIVITY_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_FT_ACTIVITY_LIST, V3270FTActivityListClass))
 #define GTK_IS_V3270_FT_ACTIVITY_LIST(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_FT_ACTIVITY_LIST))
 #define GTK_IS_V3270_FT_ACTIVITY_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_FT_ACTIVITY_LIST))
 #define GTK_V3270_FT_ACTIVITY_LIST_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_FT_ACTIVITY_LIST, V3270FTActivityListClass))

 typedef struct _V3270FTActivityList		V3270FTActivityList;
 typedef struct _V3270FTActivityListClass	V3270FTActivityListClass;

 G_GNUC_INTERNAL GtkWidget	* v3270_activity_list_new();
 G_GNUC_INTERNAL void		  v3270_activity_list_append(GtkWidget *widget, GObject *activity, gboolean select);
 G_GNUC_INTERNAL void		  v3270_activity_list_remove(GtkWidget *widget, GObject *activity);
 G_GNUC_INTERNAL void		  v3270_activity_list_load(GtkWidget *widget);
 G_GNUC_INTERNAL void		  v3270_activity_list_save(GtkWidget *widget);
 G_GNUC_INTERNAL void		  v3270_activity_list_save_as(GtkWidget *widget);

 G_GNUC_INTERNAL gchar		* v3270_select_file(GtkWidget *widget, const gchar *title, const gchar *button, GtkFileChooserAction action, const gchar *filename);

#if ! GTK_CHECK_VERSION(3,16,0)
 G_GNUC_INTERNAL void gtk_text_view_set_monospace (GtkTextView *text_view, gboolean monospace);
#endif //! GTK 3.16

 G_GNUC_INTERNAL GtkWidget	* v3270_charset_combo_box_new();


/*--[ Font Info ]------------------------------------------------------------------------------------*/

  typedef struct _v3270FontInfo {

	double					  size;						///< @brief Current font size.
	double					  step;						///< @brief Steps for zoom in/out.

	guint					  width;
	guint					  height;
	guint					  ascent;
	guint					  descent;

	struct {
		gboolean			  dynamic;
		guint				  value;
	} spacing;

	struct
	{
		gint				  left;
		gint				  top;

	} margin;

	gchar 					* family;
	cairo_font_face_t 		* face;
	cairo_font_weight_t		  weight;
	cairo_scaled_font_t		* scaled;

 } v3270FontInfo;

G_GNUC_INTERNAL void v3270_font_info_init(v3270FontInfo *info);
G_GNUC_INTERNAL void v3270_font_info_unset(v3270FontInfo *info);

G_GNUC_INTERNAL void v3270_draw_element(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *font, GdkRectangle *rect, GdkRGBA *color);

/*--[ Internal Methods ]-----------------------------------------------------------------------------*/


 const GtkWidgetClass		* v3270_get_parent_class(void);

 G_GNUC_INTERNAL void		  v3270_reconfigure(v3270 * terminal);

 G_GNUC_INTERNAL gboolean	  v3270_draw(GtkWidget * widget, cairo_t * cr);
 G_GNUC_INTERNAL void		  v3270_cursor_draw(v3270 *widget);
 G_GNUC_INTERNAL void		  v3270_set_cursor(GtkWidget *widget, LIB3270_POINTER id);

 G_GNUC_INTERNAL void		  v3270_draw_oia(v3270 *terminal, cairo_t *cr, int row, int cols);
 G_GNUC_INTERNAL void		  v3270_update_mouse_pointer(GtkWidget *widget);

 G_GNUC_INTERNAL AtkObject	* v3270_get_accessible(GtkWidget * widget);

 #if ! GTK_CHECK_VERSION(2,18,0)
	G_GNUC_INTERNAL void gtk_widget_get_allocation(GtkWidget *widget,GtkAllocation *allocation);
 #endif // !GTK(2,18)

 #if ! GTK_CHECK_VERSION(2,20,0)
	#define gtk_widget_get_realized(w)		GTK_WIDGET_REALIZED(w)
	#define gtk_widget_set_realized(w,r)	if(r) { GTK_WIDGET_SET_FLAGS(w,GTK_REALIZED); } else { GTK_WIDGET_UNSET_FLAGS(w,GTK_REALIZED); }
 #endif // !GTK(2,20)

 #if ! GTK_CHECK_VERSION(2,22,0)
	#define gtk_accessible_set_widget(a,w)	g_object_set_data(G_OBJECT(a),"widget",w)
	#define gtk_accessible_get_widget(a)	GTK_WIDGET(g_object_get_data(G_OBJECT(a),"widget"))

	G_GNUC_INTERNAL cairo_surface_t * gdk_window_create_similar_surface(GdkWindow *window, cairo_content_t content, int width, int height);

 #endif // !GTK(2,22)

 #if ! GTK_CHECK_VERSION(3,0,0)
	gboolean	  v3270_expose(GtkWidget * widget, GdkEventExpose *event);
 #endif // GTK 3

 G_GNUC_INTERNAL void		  v3270_draw_shift_status(v3270 *terminal);
 G_GNUC_INTERNAL void		  v3270_draw_alt_status(v3270 *terminal);
 G_GNUC_INTERNAL void		  v3270_draw_ins_status(v3270 *terminal);

 G_GNUC_INTERNAL void		  v3270_clear_selection(v3270 *terminal);

 G_GNUC_INTERNAL void		  v3270_update_cursor_surface(v3270 *widget, unsigned char chr, unsigned short attr);

 G_GNUC_INTERNAL void		  v3270_register_io_handlers(v3270Class *cls);

 G_GNUC_INTERNAL void 		  v3270_draw_char(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *font, GdkRectangle *rect, GdkRGBA *fg, GdkRGBA *bg);
 G_GNUC_INTERNAL void		  v3270_draw_text(cairo_t *cr, const GdkRectangle *rect, v3270FontInfo *font, const char *str);
 G_GNUC_INTERNAL void		  v3270_draw_text_at(cairo_t *cr, int x, int y, v3270FontInfo *font, const char *str);

 G_GNUC_INTERNAL void		  v3270_start_timer(GtkWidget *terminal);
 G_GNUC_INTERNAL void		  v3270_stop_timer(GtkWidget *terminal);

 G_GNUC_INTERNAL void		  v3270_redraw(v3270 *terminal, cairo_t * cr, gint width, gint height);

 G_GNUC_INTERNAL void		  v3270_draw_connection(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, const GdkRectangle *rect);

 G_GNUC_INTERNAL void		  v3270_draw_ssl_status(v3270 *widget, cairo_t *cr, GdkRectangle *rect);

 G_GNUC_INTERNAL void		  v3270_update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor);

 G_GNUC_INTERNAL void		  v3270_update_font_metrics(v3270 *terminal, unsigned int width, unsigned int height);
 G_GNUC_INTERNAL void		  v3270_compute_font_size(v3270 *terminal, cairo_t *cr, unsigned int width, unsigned int height);

 G_GNUC_INTERNAL void		  v3270_update_cursor_rect(v3270 *widget, GdkRectangle *rect, const struct v3270_character *element);

 G_GNUC_INTERNAL void		  v3270_update_message(v3270 *widget, LIB3270_MESSAGE id);
 G_GNUC_INTERNAL void		  v3270_update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
 G_GNUC_INTERNAL void		  v3270_update_oia(v3270 *terminal, LIB3270_FLAG id, unsigned char on);

 G_GNUC_INTERNAL void		  v3270_blink_ssl(v3270 *terminal);

 G_GNUC_INTERNAL void		  v3270_queue_draw_area(GtkWidget *widget, gint x, gint y, gint width, gint height);

 G_GNUC_INTERNAL void		  v3270_disable_updates(GtkWidget *widget);
 G_GNUC_INTERNAL void		  v3270_enable_updates(GtkWidget *widget);

 G_GNUC_INTERNAL void		  v3270_start_blinking(GtkWidget *widget);
 G_GNUC_INTERNAL void		  v3270_oia_update_text_field(v3270 *terminal, gboolean flag, V3270_OIA_FIELD id, const gchar chr);
 G_GNUC_INTERNAL cairo_t	* v3270_oia_set_update_region(v3270 * terminal, GdkRectangle **r, V3270_OIA_FIELD id);
 G_GNUC_INTERNAL void		  v3270_install_callbacks(v3270 *widget);

 // Keyboard & Mouse
 G_GNUC_INTERNAL gboolean	  v3270_key_press_event(GtkWidget *widget, GdkEventKey *event);
 G_GNUC_INTERNAL gboolean	  v3270_key_release_event(GtkWidget *widget, GdkEventKey *event);
 G_GNUC_INTERNAL void		  v3270_key_commit(GtkIMContext *imcontext, gchar *str, v3270 *widget);
 G_GNUC_INTERNAL gboolean	  v3270_button_press_event(GtkWidget *widget, GdkEventButton *event);
 G_GNUC_INTERNAL gboolean	  v3270_button_release_event(GtkWidget *widget, GdkEventButton*event);
 G_GNUC_INTERNAL gboolean	  v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
 G_GNUC_INTERNAL void		  v3270_emit_popup(v3270 *widget, int baddr, GdkEventButton *event);
 G_GNUC_INTERNAL gint		  v3270_get_offset_at_point(v3270 *widget, gint x, gint y);
 G_GNUC_INTERNAL gboolean	  v3270_scroll_event(GtkWidget *widget, GdkEventScroll *event);

 // I/O Callbacks
 G_GNUC_INTERNAL GSource	* IO_source_new(H3270 *session, int fd, LIB3270_IO_FLAG flag, void(*call)(H3270 *, int, LIB3270_IO_FLAG, void *), void *userdata);
 G_GNUC_INTERNAL void		  IO_source_set_state(GSource *source, gboolean enable);

 // Tools
 G_GNUC_INTERNAL void 		  v3270_translate_text_to_rgba(const gchar *colors, GdkRGBA *clr);
 G_GNUC_INTERNAL gchar		* v3270_translate_rgba_to_text(GdkRGBA *clr);

 // Properties
 G_GNUC_INTERNAL void		  v3270_init_properties(GObjectClass * gobject_class);
 G_GNUC_INTERNAL gboolean	  v3270_update_associated_lu(v3270 *terminal);
 G_GNUC_INTERNAL void		  v3270_update_toggle(GtkWidget *widget, LIB3270_TOGGLE_ID id, unsigned char value, const char *name);

 // Accelerators
 G_GNUC_INTERNAL void		  v3270_accelerator_map_sort(v3270 *widget);
 G_GNUC_INTERNAL void		  v3270_init_accelerators(v3270 *widget);

G_END_DECLS

#endif // V3270_INTERNALS_H_INCLUDED
