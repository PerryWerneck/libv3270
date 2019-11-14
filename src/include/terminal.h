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

#include <config.h>
#include <internals.h>

G_BEGIN_DECLS

 struct _v3270Class
 {
	GtkWidgetClass parent_class;

    // Internal properties.
    struct {

		size_t		  count;							// Number of properties.

		GParamSpec * font_family;
		GParamSpec * toggle[LIB3270_TOGGLE_COUNT];		// Toggle properties.

		// Signal related properties
		GParamSpec * online;
		GParamSpec * url;
		GParamSpec * associated_lu;
		GParamSpec * model;
		GParamSpec * selection;
		GParamSpec * session_name;
		GParamSpec * auto_disconnect;
		GParamSpec * remap_file;
		GParamSpec * dynamic_spacing;
		GParamSpec * lu_names;

		struct
		{
			guint toggle;
			guint boolean;
			guint integer;
			guint uint;
			guint str;
		} type;

    } properties;

    // Cursors
	GdkCursor * cursors[LIB3270_POINTER_COUNT];

	// Atoms
	GdkAtom clipboard_formatted;	///< @brief Format for internal clipboard cut & paste.

	// Signals
	void (*activate)(GtkWidget *widget);
	void (*toggle_changed)(v3270 *widget,LIB3270_TOGGLE_ID toggle_id,gboolean toggle_state,const gchar *toggle_name);
	void (*message_changed)(v3270 *widget, LIB3270_MESSAGE id);
	void (*popup_message)(GtkWidget *widget, LIB3270_NOTIFY id , const gchar *title, const gchar *message, const gchar *text);

 };

/*--[ Defines]---------------------------------------------------------------------------------------*/

 #define OIA_TOP_MARGIN 2

 #define KEY_FLAG_SHIFT	0x0001

 #ifndef WIN32
  #define KEY_FLAG_ALT	0x0002
 #endif // !WIN32

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
	int scaled_fonts	: 1;	/**< Use scaled fonts */
	int drawing			: 1;	/**< Draw widget? */

	GSource					* timer;
	GtkIMContext			* input_method;
	unsigned short			  keyflags;

	struct {

		int 				  baddr;		///< @brief Selection address.
		GdkAtom				  target;		///< @brief A GdkAtom which identifies the clipboard to use. GDK_SELECTION_CLIPBOARD gives the default clipboard.
		V3270_COPY_MODE		  format;		///< @brief Copy mode.
		GList				* blocks;		///< @brief Selection blocks.

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
	gchar					* remap_filename;				/**< @brief XML file with remap table. */

	struct {
		gchar *name;										/**< @brief Session name (for window title) */
		gchar *title;										/**< @brief Session title (for window title) */
	} session;

	// Auto disconnect
	struct
	{
		time_t					  timestamp;				/**< Last action in this widget */
		guint					  disconnect;				/**< Time (in minutes) for auto disconnect */
		GSource					* timer;					/**< Auto disconnect timer */
	} activity;

	char					  script;						///< @brief Script ID.

	// Blink
	struct
	{
		int					  show : 1;						///< @brief Show element?
		GSource				* timer;						///< @brief Timer source.
	} blink;

	GtkWidget * trace;										///< @brief Trace window handle.

 };

 G_GNUC_INTERNAL void v3270_activate(GtkWidget *widget);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

G_END_DECLS
