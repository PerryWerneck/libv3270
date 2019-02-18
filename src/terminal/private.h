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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include <config.h>

#define ENABLE_NLS
#define GETTEXT_PACKAGE PACKAGE_NAME

#include <libintl.h>
#include <glib/gi18n.h>

#include <gtk/gtk.h>
#include <v3270.h>

G_BEGIN_DECLS

 struct _v3270Class
 {
	GtkWidgetClass parent_class;

	/* Signals */
	void 		(*activate)(GtkWidget *widget);
	void 		(*toggle_changed)(v3270 *widget,LIB3270_TOGGLE toggle_id,gboolean toggle_state,const gchar *toggle_name);
	void 		(*message_changed)(v3270 *widget, LIB3270_MESSAGE id);
	void 		(*popup_message)(GtkWidget *widget, LIB3270_NOTIFY id , const gchar *title, const gchar *message, const gchar *text);

 };

/*--[ Defines]---------------------------------------------------------------------------------------*/

 #define OIA_TOP_MARGIN 2

 #define KEY_FLAG_SHIFT	0x0001

 #ifndef WIN32
  #define KEY_FLAG_ALT	0x0002
 #endif // !WIN32

 enum
 {
 	SIGNAL_TOGGLE_CHANGED,
 	SIGNAL_MESSAGE_CHANGED,
 	SIGNAL_KEYPRESS,
 	SIGNAL_CONNECTED,
 	SIGNAL_DISCONNECTED,
 	SIGNAL_UPDATE_CONFIG,
 	SIGNAL_MODEL_CHANGED,
 	SIGNAL_SELECTING,
 	SIGNAL_POPUP,
 	SIGNAL_PASTENEXT,
 	SIGNAL_CLIPBOARD,
 	SIGNAL_CHANGED,
 	SIGNAL_MESSAGE,
 	SIGNAL_FIELD,
 	SIGNAL_PRINT_SETUP,
 	SIGNAL_PRINT_APPLY,
 	SIGNAL_PRINT_DONE,

 	LAST_SIGNAL
 };


/*--[ Globals ]--------------------------------------------------------------------------------------*/

 struct v3270_ssl_status_msg
 {
	long			  id;
	const gchar		* icon;
	const gchar		* text;
	const gchar		* message;
 };

/*--[ Widget data ]----------------------------------------------------------------------------------*/

 struct _v3270
 {
	GtkWidget parent;

	// flags
	int selecting		: 1;	/**< Selecting region */
	int moving			: 1;	/**< Moving selected region */
	int resizing		: 1;	/**< Resizing selected region */
	int table			: 1;	/**< Copy mode is table */
	int scaled_fonts	: 1;	/**< Use scaled fonts */
	int drawing			: 1;	/**< Draw widget? */

#if GTK_CHECK_VERSION(3,0,0)

#else
    gint width;
    gint height;
#endif // GTK_CHECK_VERSION(3,0,0)

	GSource					* timer;
	GtkIMContext			* input_method;
	unsigned short			  keyflags;

	struct
	{
	    char                * text;                 	/**< Clipboard contents (lib3270 charset) */
        int                   baddr;	        		/**< Selection addr */
	} selection;

	LIB3270_POINTER 		  pointer_id;
	unsigned char			  pointer;					/**< Mouse pointer ID */

	// Font info
	cairo_surface_t			* surface;
	v3270FontInfo			  font;

	gint     				  minimum_width;
	gint					  minimum_height;

	// Colors
	GdkRGBA					  color[V3270_COLOR_COUNT];		/**< Terminal widget colors */

	// OIA
	struct
	{
		GdkRectangle		  rect[V3270_OIA_FIELD_COUNT];
		V3270_OIA_FIELD		  selected;						/**< Clicked OIA field */
	} oia;

	struct
	{
		unsigned char 		  show;							/**< Cursor flag */
		unsigned char 		  chr;							/**< Char at cursor position */
		unsigned short 		  attr;							/**< Attribute at cursor position */
		GdkRectangle		  rect;							/**< Cursor rectangle */
		GSource				* timer;						/**< Cursor blinking timer */
		cairo_surface_t		* surface;						/**< Cursor image */
	} cursor;

	// Acessibility
	GtkAccessible			* accessible;

	// Session
	H3270   				* host;							/**< Related 3270 session */
	gchar					* session_name;					/**< Session name (for window title) */

	// Auto disconnect
	struct
	{
		time_t					  timestamp;				/**< Last action in this widget */
		guint					  disconnect;				/**< Time (in minutes) for auto disconnect */
		GSource					* timer;					/**< Auto disconnect timer */
	} activity;

	char					  script;						/**< @brief Script ID */

	// Blink
	struct
	{
		int					  show : 1;						/**< @brief Show element? */
		GSource				* timer;						/**< @brief Timer source. */
	} blink;

	/*
	// Scripting
	struct
	{
		int					  blink : 1;
		gchar				  id;						///< Script indicator
		GSource				* timer;
	} script;
	*/

 };

/*--[ Properties ]-----------------------------------------------------------------------------------*/

 /*
 enum
 {
	PROP_0,

	// Construct
	PROP_TYPE,

	// Widget properties
	PROP_ONLINE,
	PROP_SELECTION,
	PROP_MODEL,
	PROP_LUNAME,
	PROP_AUTO_DISCONNECT,
	PROP_URL,
	PROP_SESSION_NAME,

	// Toggles - always the last one, the real values are PROP_TOGGLE+LIB3270_TOGGLE
	PROP_TOGGLE
 };

 #define PROP_LAST (PROP_TOGGLE+LIB3270_TOGGLE_COUNT)
 */

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 G_GNUC_INTERNAL guint			  v3270_widget_signal[LAST_SIGNAL];
 G_GNUC_INTERNAL GdkCursor		* v3270_cursor[LIB3270_POINTER_COUNT];
 G_GNUC_INTERNAL const gchar	* v3270_default_colors;
 G_GNUC_INTERNAL const gchar	* v3270_default_font;
 G_GNUC_INTERNAL void			  v3270_start_blinking(GtkWidget *widget);

 G_GNUC_INTERNAL void			  v3270_oia_update_text_field(v3270 *terminal, gboolean flag, V3270_OIA_FIELD id, const gchar chr);
 G_GNUC_INTERNAL cairo_t		* v3270_oia_set_update_region(v3270 * terminal, GdkRectangle **r, V3270_OIA_FIELD id);

 G_GNUC_INTERNAL void			  v3270_install_callbacks(v3270 *widget);

 G_GNUC_INTERNAL struct _v3270_properties
 {
    size_t		  count;							// Number of properties.
    GParamSpec	* toggle[LIB3270_TOGGLE_COUNT];		// Toggle properties.

    struct
    {
    	guint toggle;
    	guint boolean;
    	guint integer;
    	guint str;
    } type;

    GParamSpec * online;
    GParamSpec * luname;
    GParamSpec * model;
    GParamSpec * selection;

 } v3270_properties;


/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

const GtkWidgetClass		* v3270_get_parent_class(void);

G_GNUC_INTERNAL gboolean	  v3270_draw(GtkWidget * widget, cairo_t * cr);
G_GNUC_INTERNAL void		  v3270_cursor_draw(v3270 *widget);

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

G_GNUC_INTERNAL void		  v3270_clear_clipboard(v3270 *terminal);

G_GNUC_INTERNAL void		  v3270_update_cursor_surface(v3270 *widget,unsigned char chr,unsigned short attr);

G_GNUC_INTERNAL void		  v3270_register_io_handlers(v3270Class *cls);

G_GNUC_INTERNAL void 		  v3270_draw_char(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *font, GdkRectangle *rect, GdkRGBA *fg, GdkRGBA *bg);
G_GNUC_INTERNAL void		  v3270_draw_text(cairo_t *cr, const GdkRectangle *rect, v3270FontInfo *font, const char *str);
G_GNUC_INTERNAL void		  v3270_draw_text_at(cairo_t *cr, int x, int y, v3270FontInfo *font, const char *str);

G_GNUC_INTERNAL void		  v3270_start_timer(GtkWidget *terminal);
G_GNUC_INTERNAL void		  v3270_stop_timer(GtkWidget *terminal);

G_GNUC_INTERNAL void		  v3270_draw_connection(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, const GdkRectangle *rect);

G_GNUC_INTERNAL void		  v3270_draw_ssl_status(v3270 *widget, cairo_t *cr, GdkRectangle *rect);

G_GNUC_INTERNAL void		  v3270_update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor);

G_GNUC_INTERNAL void		  v3270_update_font_metrics(v3270 *terminal, cairo_t *cr, int width, int height);

G_GNUC_INTERNAL void		  v3270_update_cursor_rect(v3270 *widget, GdkRectangle *rect, unsigned char chr, unsigned short attr);

G_GNUC_INTERNAL void		  v3270_update_message(v3270 *widget, LIB3270_MESSAGE id);
G_GNUC_INTERNAL void		  v3270_update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
G_GNUC_INTERNAL void		    v3270_update_oia(v3270 *terminal, LIB3270_FLAG id, unsigned char on);

G_GNUC_INTERNAL void			v3270_blink_ssl(v3270 *terminal);

G_GNUC_INTERNAL void			v3270_update_luname(GtkWidget *widget,const gchar *name);
G_GNUC_INTERNAL void			v3270_init_properties(GObjectClass * gobject_class);
G_GNUC_INTERNAL	void			v3270_queue_draw_area(GtkWidget *widget, gint x, gint y, gint width, gint height);

G_GNUC_INTERNAL void			v3270_disable_updates(GtkWidget *widget);
G_GNUC_INTERNAL void			v3270_enable_updates(GtkWidget *widget);

// Keyboard & Mouse
G_GNUC_INTERNAL gboolean		v3270_key_press_event(GtkWidget *widget, GdkEventKey *event);
G_GNUC_INTERNAL gboolean		v3270_key_release_event(GtkWidget *widget, GdkEventKey *event);
G_GNUC_INTERNAL void			v3270_key_commit(GtkIMContext *imcontext, gchar *str, v3270 *widget);
G_GNUC_INTERNAL gboolean		v3270_button_press_event(GtkWidget *widget, GdkEventButton *event);
G_GNUC_INTERNAL gboolean		v3270_button_release_event(GtkWidget *widget, GdkEventButton*event);
G_GNUC_INTERNAL gboolean		v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
G_GNUC_INTERNAL void			v3270_emit_popup(v3270 *widget, int baddr, GdkEventButton *event);
G_GNUC_INTERNAL gint			v3270_get_offset_at_point(v3270 *widget, gint x, gint y);
G_GNUC_INTERNAL gboolean		v3270_scroll_event(GtkWidget *widget, GdkEventScroll *event);

// I/O Callbacks
G_GNUC_INTERNAL GSource			* IO_source_new(H3270 *session, int fd, LIB3270_IO_FLAG flag, void(*call)(H3270 *, int, LIB3270_IO_FLAG, void *), void *userdata);
G_GNUC_INTERNAL void			  IO_source_set_state(GSource *source, gboolean enable);

G_END_DECLS
