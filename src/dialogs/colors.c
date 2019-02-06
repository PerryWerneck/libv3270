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

 #include "private.h"
 #include <v3270/colorscheme.h>
 #include <lib3270/log.h>

 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270ColorSelectionClass
 {
 	GtkGridClass parent_class;

 };

 struct _V3270ColorSelection
 {
 	GtkGrid	  parent;

 	GtkWidget			* terminal;
	int 			  	  selected;

	struct {
		GtkWidget		* view;
		GtkTreeModel 	* list;
#if USE_GTK_COLOR_CHOOSER
		GtkWidget 		* chooser;
#else
		GtkWidget		* editor;
#endif // USE_GTK_COLOR_CHOOSER
		GtkWidget 		* scheme;
	} colors;

	/// @brief Saved colors.
	GdkRGBA			  saved[V3270_COLOR_COUNT];

 };

 G_DEFINE_TYPE(V3270ColorSelection, V3270ColorSelection, GTK_TYPE_GRID);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270ColorSelection_class_init(G_GNUC_UNUSED V3270ColorSelectionClass *klass)
 {


 }

 static void update_color_chooser(V3270ColorSelection *widget, int id)
 {
	widget->selected = id;

	if(id < 0 || id >= V3270_COLOR_COUNT)
	{
#if USE_GTK_COLOR_CHOOSER
		gtk_widget_set_sensitive(widget->colors.chooser,FALSE);
#else
		gtk_widget_set_sensitive(widget->colors.editor,FALSE);
#endif // USE_GTK_COLOR_CHOOSER
		return;
	}

	GdkRGBA	* clr = v3270_get_color(widget->terminal,id);

#if USE_GTK_COLOR_CHOOSER
    {
        GValue value;

        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widget->colors.chooser),clr);

        g_value_init(&value, G_TYPE_BOOLEAN);
        g_value_set_boolean(&value,FALSE);
        g_object_set_property(G_OBJECT(widget->colors.chooser),"show-editor",&value);

		gtk_widget_set_sensitive(widget->colors.chooser,TRUE);

    }
#else

	gtk_color_selection_set_previous_rgba(GTK_COLOR_SELECTION(widget->colors.editor),widget->saved+id);
	gtk_color_selection_set_current_rgba(GTK_COLOR_SELECTION(widget->colors.editor),clr);

	gtk_widget_set_sensitive(widget->colors.editor,TRUE);
#endif // GTK_CHECK_VERSION

 }

 static void color_scheme_changed(GtkWidget G_GNUC_UNUSED(*dunno), const GdkRGBA *colors, V3270ColorSelection *widget) {

	debug("%s=%p",__FUNCTION__,colors);

	int f;
	for(f=0;f<V3270_COLOR_COUNT;f++)
		v3270_set_color(widget->terminal,f,colors+f);

	update_color_chooser(widget,widget->selected);

	v3270_reload(widget->terminal);
	gtk_widget_queue_draw(widget->terminal);

 }

 LIB3270_EXPORT void v3270_color_selection_reset(GtkWidget *widget)
 {
	V3270ColorSelection * sel = GTK_V3270_COLOR_SELECTION(widget);

	int f;
	for(f=0;f<V3270_COLOR_COUNT;f++)
		v3270_set_color(sel->terminal,f,sel->saved+f);

	update_color_chooser(sel,sel->selected);

	v3270_reload(sel->terminal);
	gtk_widget_queue_draw(sel->terminal);

 }


#if USE_GTK_COLOR_CHOOSER
 static void color_activated(GtkColorChooser *chooser, GdkRGBA *clr, V3270ColorSelection *widget)
 {
	if(widget->selected < 0 || widget->selected >= V3270_COLOR_COUNT)
		return;

    trace("Updating color %d",widget->selected);

	v3270_set_color(widget->terminal,widget->selected,clr);
	v3270_reload(widget->terminal);
	v3270_color_scheme_set_rgba(widget->colors.scheme,v3270_get_color_table(widget->terminal));
	gtk_widget_queue_draw(widget->terminal);

 }
#else
 static void color_changed(GtkWidget *colorselection, V3270ColorSelection *widget)
 {
 	GdkRGBA	clr;

	if(widget->selected < 0 || widget->selected >= V3270_COLOR_COUNT)
		return;

	gtk_color_selection_get_current_rgba(GTK_COLOR_SELECTION(colorselection),&clr);

	v3270_set_color(widget->terminal,widget->selected,&clr);
	v3270_reload(widget->terminal);
	v3270_color_scheme_set_rgba(widget->colors.scheme,v3270_get_color_table(widget->terminal));
	gtk_widget_queue_draw(widget->terminal);

 }
#endif // GTK_CHECK_VERSION

 static void color_selected(GtkTreeSelection *selection, V3270ColorSelection *widget)
 {
	GValue			  value		= { 0, };
	GtkTreeModel	* model;
	GtkTreeIter		  iter;

	if(!gtk_tree_selection_get_selected(selection,&model,&iter))
		return;

	gtk_tree_model_get_value(model,&iter,1,&value);

	update_color_chooser(widget,g_value_get_int(&value));

 }


 static void V3270ColorSelection_init(V3270ColorSelection *widget)
 {
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

	{
		// Create colors list view.
		widget->colors.list = (GtkTreeModel *) gtk_tree_store_new(2,G_TYPE_STRING,G_TYPE_INT);
		widget->colors.view = gtk_tree_view_new_with_model(widget->colors.list);

		gtk_widget_set_tooltip_markup(widget->colors.view,_("Terminal colors"));

		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget->colors.view),FALSE);
		gtk_tree_view_insert_column_with_attributes(	GTK_TREE_VIEW(widget->colors.view),
														-1,
														"color",gtk_cell_renderer_text_new(),"text",
														0, NULL );

		// Setup selection mode.
		GtkTreeSelection * select = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget->colors.view));
		gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
		g_signal_connect(G_OBJECT(select),"changed",G_CALLBACK(color_selected),widget);

		// Put the view inside a scrolled window.
		GtkWidget * box	= gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
		gtk_container_add(GTK_CONTAINER(box),widget->colors.view);

		gtk_widget_set_vexpand(box,TRUE);
		gtk_widget_set_hexpand(box,TRUE);

		gtk_grid_attach(GTK_GRID(widget),box,0,0,1,5);

	}

#ifdef USE_GTK_COLOR_CHOOSER
	{
		// Create color chooser widget
		widget->colors.chooser = gtk_color_chooser_widget_new();

		gtk_widget_set_sensitive(widget->colors.chooser,0);
		g_signal_connect(G_OBJECT(widget->colors.chooser),"color-activated",G_CALLBACK(color_activated),widget);

		gtk_grid_attach(GTK_GRID(widget),widget->colors.chooser,1,0,5,5);
	}
#else
	{
		widget->colors.editor = gtk_color_selection_new();
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(widget->colors.editor),FALSE);
		gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(widget->colors.editor),TRUE);

		gtk_widget_set_sensitive(widget->colors.editor,0);
		g_signal_connect(G_OBJECT(widget->colors.editor),"color-changed",G_CALLBACK(color_changed),widget);

		gtk_grid_attach(GTK_GRID(widget),widget->colors.editor,1,0,5,5);
	}
#endif // USE_GTK_COLOR_CHOOSER

	{
		// Create scheme selector
		widget->colors.scheme = v3270_color_scheme_new();
		gtk_widget_set_tooltip_markup(widget->colors.scheme,_("Predefined color schemes"));
		gtk_grid_attach(GTK_GRID(widget),widget->colors.scheme,0,6,6,1);

		g_signal_connect(G_OBJECT(widget->colors.scheme),"update-colors",G_CALLBACK(color_scheme_changed),widget);

	}


 }

 LIB3270_EXPORT GtkWidget * v3270_color_selection_new(GtkWidget *terminal)
 {
 	g_return_val_if_fail(GTK_IS_V3270(terminal),NULL);

	V3270ColorSelection * widget = GTK_V3270_COLOR_SELECTION(g_object_new(GTK_TYPE_V3270_COLOR_SELECTION, NULL));

	widget->terminal = terminal;

	// Load colors
	{
		static const struct _node
		{
			int			id;
			const char	*text;
		} node[] =
		{
			{ V3270_COLOR_BACKGROUND,		N_( "Terminal colors" )		},
			{ V3270_COLOR_FIELD,			N_( "Field colors" )		},
			{ V3270_COLOR_SELECTED_BG,		N_( "Misc colors" )			},
		};

		static const gchar *color_name[V3270_COLOR_COUNT] =
		{
			N_( "Background" ),					// V3270_COLOR_BACKGROUND
			N_( "Blue" ),						// V3270_COLOR_BLUE
			N_( "Red" ),						// V3270_COLOR_RED
			N_( "Pink" ),						// V3270_COLOR_PINK
			N_( "Green" ),						// V3270_COLOR_GREEN
			N_( "Turquoise" ),					// V3270_COLOR_TURQUOISE
			N_( "Yellow" ),						// V3270_COLOR_YELLOW
			N_( "White" ),						// V3270_COLOR_WHITE
			N_( "Black" ),						// V3270_COLOR_BLACK
			N_( "Dark Blue" ),					// V3270_COLOR_DARK_BLUE
			N_( "Orange" ),						// V3270_COLOR_ORANGE
			N_( "Purple" ),						// V3270_COLOR_PURPLE
			N_( "Dark Green" ),					// V3270_COLOR_DARK_GREEN
			N_( "Dark Turquoise" ),				// V3270_COLOR_DARK_TURQUOISE
			N_( "Mustard" ),					// V3270_COLOR_MUSTARD
			N_( "Gray" ),						// V3270_COLOR_GRAY

			N_( "Normal/Unprotected" ),			// V3270_COLOR_FIELD
			N_( "Intensified/Unprotected" ),	// V3270_COLOR_FIELD_INTENSIFIED
			N_( "Normal/Protected" ),			// V3270_COLOR_FIELD_PROTECTED
			N_( "Intensified/Protected" ),		// V3270_COLOR_FIELD_PROTECTED_INTENSIFIED

			N_( "Selection background" ),		// TERMINAL_COLOR_SELECTED_BG
			N_( "Selection foreground" ),		// TERMINAL_COLOR_SELECTED_FG

			N_( "Cross hair cursor" ),			// TERMINAL_COLOR_CROSS_HAIR

			// Oia Colors
			N_( "OIA background" ),				// TERMINAL_COLOR_OIA_BACKGROUND
			N_( "OIA foreground" ),				// TERMINAL_COLOR_OIA_FOREGROUND
			N_( "OIA separator" ),				// TERMINAL_COLOR_OIA_SEPARATOR
			N_( "OIA status ok" ),				// TERMINAL_COLOR_OIA_STATUS_OK
			N_( "OIA Warning"	),				// V3270_COLOR_OIA_STATUS_WARNING
			N_( "OIA status invalid" ),			// TERMINAL_COLOR_OIA_STATUS_INVALID

		};

		GtkTreeIter	iter;
		GtkTreeIter	parent;
		int			title = 0;
		int 		f;

		gtk_tree_store_append(GTK_TREE_STORE(widget->colors.list),&parent,NULL);
		gtk_tree_store_set(GTK_TREE_STORE(widget->colors.list), &parent, 0, gettext(node[title++].text), 1, V3270_COLOR_COUNT, -1);

		for(f=0;f<V3270_COLOR_COUNT;f++)
		{
			widget->saved[f] = *(v3270_get_color(terminal,f));

			if(f == node[title].id)
			{
				gtk_tree_store_append(GTK_TREE_STORE(widget->colors.list),&parent,NULL);
				gtk_tree_store_set(GTK_TREE_STORE(widget->colors.list), &parent, 0, gettext(node[title++].text), 1, V3270_COLOR_COUNT, -1);
			}
			gtk_tree_store_append(GTK_TREE_STORE(widget->colors.list),&iter,&parent);
			gtk_tree_store_set(GTK_TREE_STORE(widget->colors.list), &iter, 0, gettext(color_name[f]), 1, f, -1);
		}

		gtk_tree_view_expand_all(GTK_TREE_VIEW(widget->colors.view));

	}

	v3270_color_scheme_set_rgba(widget->colors.scheme,v3270_get_color_table(terminal));

	gtk_widget_show_all(GTK_WIDGET(widget));
 	return GTK_WIDGET(widget);
 }

