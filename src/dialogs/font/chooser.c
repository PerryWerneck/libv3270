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

 /**
  * @brief Implements the V3270 Font Chooser widget.
  *
  * Reference:
  *
  */

 #include "../private.h"
 #include <v3270/dialogs.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270_FONT_CHOOSER			(V3270FontChooserWidget_get_type ())
 #define GTK_V3270_FONT_CHOOSER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_FONT_CHOOSER, V3270FontChooserWidget))
 #define GTK_V3270_FONT_CHOOSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_FONT_CHOOSER, V3270FontChooserWidgetClass))
 #define GTK_IS_V3270_FONT_CHOOSER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_FONT_CHOOSER))
 #define GTK_IS_V3270_FONT_CHOOSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_FONT_CHOOSER))
 #define GTK_V3270_FONT_CHOOSER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_FONT_CHOOSER, V3270FontChooserWidgetClass))

 typedef struct _V3270FontChooserWidget
 {
 	V3270Settings	  parent;

 	GtkWidget		* font_list;
 	GtkWidget		* preview;

 } V3270FontChooserWidget;

 typedef struct _V3270HostSelectWidgetClass
 {
	V3270SettingsClass parent_class;
 } V3270FontChooserWidgetClass;


 G_DEFINE_TYPE(V3270FontChooserWidget, V3270FontChooserWidget, GTK_TYPE_V3270_SETTINGS);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void apply(GtkWidget *widget, GtkWidget *terminal)
{

	debug("V3270FontChooserWidget::%s",__FUNCTION__);

}

static void load(GtkWidget *widget, GtkWidget *terminal)
{
	V3270FontChooserWidget *chooser = GTK_V3270_FONT_CHOOSER(widget);

	debug("V3270FontChooserWidget::%s",__FUNCTION__);

	GtkTreeIter active;
	gtk_tree_view_set_model(
			GTK_TREE_VIEW(chooser->font_list),
			v3270_font_family_model_new(
						chooser->font_list,
						v3270_get_font_family(terminal),
						&active
				)
	);

	gtk_tree_selection_select_iter(
		gtk_tree_view_get_selection(GTK_TREE_VIEW(chooser->font_list)),
		&active
	);

}

static void V3270FontChooserWidget_class_init(V3270FontChooserWidgetClass *klass)
{
	V3270SettingsClass * widget = GTK_V3270_SETTINGS_CLASS(klass);

	widget->apply = apply;
	widget->load = load;

}

 static void font_selected(GtkTreeSelection *selection, V3270FontChooserWidget *widget)
 {
	GValue			  value		= { 0, };
	GtkTreeModel	* model;
	GtkTreeIter		  iter;

	if(!gtk_tree_selection_get_selected(selection,&model,&iter))
		return;

	gtk_tree_model_get_value(model,&iter,0,&value);

	debug("Font-family: %s",g_value_get_string(&value));

	// Update terminal widget
	GtkWidget * terminal = v3270_settings_get_terminal_widget(GTK_WIDGET(widget));
	if(terminal)
		v3270_set_font_family(terminal,g_value_get_string(&value));

	g_value_unset(&value);
 }

static void V3270FontChooserWidget_init(V3270FontChooserWidget *widget)
{
	gtk_widget_set_size_request(GTK_WIDGET(widget),-1,136);
	gtk_grid_set_row_homogeneous(GTK_GRID(widget),FALSE);

	// Create font list view
	{
		widget->font_list = gtk_tree_view_new();

		gtk_widget_set_tooltip_markup(widget->font_list,_("Available fonts"));

		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget->font_list),FALSE);
		gtk_tree_view_insert_column_with_attributes(	GTK_TREE_VIEW(widget->font_list),
														-1,
														"text",gtk_cell_renderer_text_new(),"text",
														0, NULL
													);

		// Setup selection mode.
		GtkTreeSelection * select = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget->font_list));
		gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
		g_signal_connect(G_OBJECT(select),"changed",G_CALLBACK(font_selected),widget);

		// Put the view inside a scrolled window.
		GtkWidget * box	= gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
		gtk_container_add(GTK_CONTAINER(box),widget->font_list);

		gtk_widget_set_vexpand(box,TRUE);
		gtk_widget_set_hexpand(box,FALSE);

		gtk_grid_attach(GTK_GRID(widget),box,0,0,1,5);
	}

	// Add preview widgets
	{
		widget->preview = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(widget->preview),pango_language_get_sample_string(NULL));

		gtk_widget_set_can_default(widget->preview,FALSE);
		gtk_widget_set_can_focus(widget->preview,FALSE);
		gtk_editable_set_editable(GTK_EDITABLE(widget->preview),FALSE);

		gtk_widget_set_vexpand(widget->preview,FALSE);
		gtk_widget_set_hexpand(widget->preview,TRUE);

		gtk_grid_attach(GTK_GRID(widget),widget->preview,1,0,5,1);
	}

}

GtkWidget * v3270_font_chooser_widget_new()
{
	V3270FontChooserWidget * font_chooser = (V3270FontChooserWidget *) g_object_new(GTK_TYPE_V3270_FONT_CHOOSER, NULL);


	return GTK_WIDGET(font_chooser);
}

