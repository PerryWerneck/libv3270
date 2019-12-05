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

 /// @brief V3270 Properties saved to the configuration file.
 typedef enum
 {
	V3270_SETTING_URL,
	V3270_SETTING_FONT_FAMILY,
	V3270_SETTING_AUTO_DISCONNECT,
	V3270_SETTING_REMAP_FILE,
	V3270_SETTING_DYNAMIC_SPACING,
	V3270_SETTING_LU_NAMES,
	V3270_SETTING_MODEL_NUMBER,
	V3270_SETTING_OVERSIZE,
	V3270_SETTING_HOST_CHARSET,
	V3270_SETTING_UNLOCK_DELAY,
	V3270_SETTING_COLOR_TYPE,
	V3270_SETTING_HOST_TYPE,
	V3270_SETTING_CRL_PROTOCOL,
	V3270_SETTING_TERMINAL_COLORS,
	V3270_SETTING_SELECTION_OPTIONS,

 	V3270_SETTING_COUNT					///< @brief Number of setting properties.
 } V3270_SETTING;

 G_GNUC_INTERNAL void v3270_notify_setting(GtkWidget *widget, V3270_SETTING id);

 struct _v3270Class
 {
	GtkWidgetClass parent_class;

    // Internal properties.
    struct {

		size_t		  count;							// Number of properties.

		GParamSpec * toggle[LIB3270_TOGGLE_COUNT];		// Toggle properties.

		// Properties saved to the configuration file.
		GParamSpec * settings[V3270_SETTING_COUNT];

		// Signal related properties
		GParamSpec * online;
		GParamSpec * associated_lu;
		GParamSpec * model;
		GParamSpec * selection;
		GParamSpec * session_name;
		GParamSpec * trace;

		struct
		{
			guint toggle;
			guint boolean;
			guint integer;
			guint uint;
			guint str;
			guint responses;
		} type;

    } properties;

    // Predefined responses.
   	GParamSpec * responses[V3270_TOGGLEABLE_DIALOG_CUSTOM];

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

 typedef enum _V3270SelectionOption {

 	V3270_SELECTION_PLAIN_TEXT		= 0x0000,	///< @brief Uses only plain text.
 	V3270_SELECTION_FONT_FAMILY		= 0x0001,	///< @brief Inform font-family.
 	V3270_SELECTION_COLORS			= 0x0002,	///< @brief Inform terminal colors.

 } V3270SelectionOption;

 #define V3270_SELECTION_DEFAULT (V3270_SELECTION_FONT_FAMILY|V3270_SELECTION_COLORS)

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
	int selecting		: 1;	/// @brief Selecting region
	int moving			: 1;	/// @brief Moving selected region
	int resizing		: 1;	/// @brief Resizing selected region
	int scaled_fonts	: 1;	/// @brief Use scaled fonts
	int drawing			: 1;	/// @brief Draw widget?
	int freeze			: 1;	/// @brief Truee when the "save settings" signal is disabled.

    /// @brief Action properties.
   	GtkResponseType			  responses[V3270_TOGGLEABLE_DIALOG_CUSTOM];

	GSource					* timer;
	GtkIMContext			* input_method;
	unsigned short			  keyflags;

	struct {

		int 					  baddr;		///< @brief Selection address.
		GdkAtom					  target;		///< @brief A GdkAtom which identifies the clipboard to use. GDK_SELECTION_CLIPBOARD gives the default clipboard.
		V3270_COPY_MODE			  format;		///< @brief Copy mode.
		GList					* blocks;		///< @brief Selection blocks.
		V3270SelectionOption	  options;		///< @brief Selection options.

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

	// Keyboard accelerators.
	GSList * accelerators;									///< @brief Keyboard accelerators.

	// Scroll actions.
	struct
	{
		void (*activate)(GtkWidget *, gpointer);			///< @brief Callback for scroll activation.
		gpointer arg;										///< @brief Argument for callback.
	} scroll[4];											///< @brief Scroll actions.

 };

 G_GNUC_INTERNAL void v3270_activate(GtkWidget *widget);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

G_END_DECLS
