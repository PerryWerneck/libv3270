/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <internals.h>
 #include <v3270/selection.h>

 G_BEGIN_DECLS

 struct _v3270Class
 {
	GtkWidgetClass parent_class;

    // Internal properties.
    struct {

		size_t		  count;							// Number of properties.

		GParamSpec * toggle[LIB3270_TOGGLE_COUNT];		// Toggle properties.

		// Properties saved to the configuration file.
		const gchar **persistent;

		// Signal related properties
		GParamSpec * online;
		GParamSpec * associated_lu;
		GParamSpec * selection;
		GParamSpec * session_name;
		GParamSpec * trace;
		GParamSpec * has_copy;
		GParamSpec * has_timer;

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
	guint (*load_popup_response)(v3270 *widget, const gchar *popup_name, guint response);
	gboolean (*save_popup_response)(v3270 *widget, const gchar *popup_name, guint response);

 };

/*--[ Defines]---------------------------------------------------------------------------------------*/

 #define OIA_TOP_MARGIN 2

 #define KEY_FLAG_SHIFT	0x0001

 #ifndef WIN32
  #define KEY_FLAG_ALT	0x0002
 #endif // !WIN32

 typedef enum _V3270SelectionOption {

 	V3270_SELECTION_PLAIN_TEXT			= 0x00,	///< @brief Uses only plain text.
 	V3270_SELECTION_FONT_FAMILY			= 0x01,	///< @brief Inform font-family.
 	V3270_SELECTION_COLORS				= 0x02,	///< @brief Inform terminal colors.
 	V3270_SELECTION_NON_BREAKABLE_SPACE	= 0x04,	///< @brief Use non breakable spaces.
 	V3270_SELECTION_SCREEN_PASTE		= 0x08,	///< @brief Enable screen paste.
 	V3270_SELECTION_SMART_COPY			= 0x10,	///< @brief Enable copy/append based on current selection state.
 	V3270_SELECTION_DIALOG_STATE		= 0x20,	///< @brief Used for settings dialog.
 	V3270_SELECTION_PIXBUFF				= 0x40,	///< @brief Allow pixbuf formats.

 } V3270SelectionOption;

 #define V3270_SELECTION_DEFAULT		(V3270_SELECTION_FONT_FAMILY|V3270_SELECTION_COLORS|V3270_SELECTION_NON_BREAKABLE_SPACE|V3270_SELECTION_DIALOG_STATE)
 #define V3270_SELECTION_ENABLE_HTML	(V3270_SELECTION_FONT_FAMILY|V3270_SELECTION_COLORS|V3270_SELECTION_NON_BREAKABLE_SPACE)

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
	unsigned int selecting		: 1;	///< @brief Selecting region
	unsigned int moving			: 1;	///< @brief Moving selected region
	unsigned int resizing		: 1;	///< @brief Resizing selected region
	unsigned int scaled_fonts	: 1;	///< @brief Use scaled fonts
	unsigned int drawing		: 1;	///< @brief Draw widget?
	unsigned int freeze			: 1;	///< @brief True when the "save settings" signal is disabled.
	unsigned int append			: 1;	///< @brief Next smart-copy operation will be append.
	unsigned int copying		: 1;	///< @brief Copy with center mouse button
	unsigned int open_url		: 1;	///< @brief Emit 'open-url' signal when and http:// or https:// url is selected by 'word-select' action.

    /// @brief Action properties.
//   	GtkResponseType			  responses[V3270_TOGGLEABLE_DIALOG_CUSTOM];

	GSource					* timer;
	GtkIMContext			* input_method;
	unsigned short			  keyflags;

	struct {

		int 					  baddr;		///< @brief Selection address.
		GdkAtom					  target;		///< @brief A GdkAtom which identifies the clipboard to use. GDK_SELECTION_CLIPBOARD gives the default clipboard.
		V3270_COPY_MODE			  format;		///< @brief Copy mode.
		GList					* blocks;		///< @brief Selection blocks.
		V3270SelectionOption	  options;		///< @brief Selection options.
		gchar					* font_family;	///< @brief Font family for HTML paste.

		struct {
			gchar * scheme;						///< @brief Name of the selection color scheme
			gchar * value;						///< @brief List of colors for selection.
		} color;

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
		unsigned int		  show : 1;						///< @brief Show element?
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
 G_GNUC_INTERNAL GdkPixbuf * v3270_get_as_pixbuf(GtkWidget *widget);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

G_END_DECLS
