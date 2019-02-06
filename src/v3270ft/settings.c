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

 #include <string.h>
 #include <v3270.h>
 #include <v3270/filetransfer.h>
 #include "private.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270FTSettingsClass
 {
 	GtkGridClass parent_class;

 };

 struct _V3270FTSettings
 {
 	GtkGrid parent;

 	struct {
 		GtkEntry * local;
 		GtkEntry * remote;
 	} file;

 	GtkWidget * options[NUM_OPTIONS_WIDGETS];
 };

 G_DEFINE_TYPE(V3270FTSettings, V3270FTSettings, GTK_TYPE_GRID);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270FTSettings_class_init(G_GNUC_UNUSED V3270FTSettingsClass *klass)
 {


 }

 static GtkWidget * create_label(V3270FTSettings *widget, const gchar *mnemonic, gint left, gint top)
 {
	GtkWidget * label = gtk_label_new_with_mnemonic(mnemonic);
	gtk_widget_set_halign(GTK_WIDGET(label),GTK_ALIGN_END);
	gtk_grid_attach(GTK_GRID(widget),label,left,top,1,1);
	return label;
 }

 static GtkWidget * create_entry(V3270FTSettings *widget, const gchar *mnemonic, GtkWidget *entry, gint left, gint top, gint width)
 {
 	GtkWidget * label = create_label(widget,mnemonic,left,top);
	gtk_grid_attach(GTK_GRID(widget),entry,left+1,top,width,1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label),entry);
	gtk_widget_set_hexpand(entry,TRUE);
	return entry;
 }

 static GtkWidget * create_frame(V3270FTSettings *widget, const gchar *title, GtkWidget *box, gint left, gint top, gint width, gint height, gint margin_top)
 {
	GtkFrame			* frame		= GTK_FRAME(gtk_frame_new(""));
	g_autofree gchar	* markup	= g_strdup_printf("<b>%s</b>",title);
	GtkWidget			* label		= gtk_label_new(NULL);

	if(margin_top)
		g_object_set(G_OBJECT(frame),"margin-top",margin_top,NULL);

	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_label_set_markup(GTK_LABEL(label),markup);
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);

	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(box));

	gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(frame),left,top,width,height);

	g_object_set(G_OBJECT(box),"margin-top",6,NULL);

	return box;
 }

 static void V3270FTSettings_init(V3270FTSettings *widget)
 {
 	size_t ix;

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

 	// Operation type
 	{
 		GtkTreeModel	* model = GTK_TREE_MODEL(gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_ULONG));
		GtkWidget		* entry = create_entry(widget,"_Operation",gtk_combo_box_new_with_model(model),0,0,10);
		GtkCellRenderer	* renderer = gtk_cell_renderer_text_new();

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(entry), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(entry), renderer, "text", 0, NULL);

		for(ix=0;ix < NUM_TYPES;ix++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(ft_type[ix].label),-1);
		}


 	}

 	// Local and remote file names.
	{
		// Local file name
		widget->file.local = GTK_ENTRY(create_entry(widget,"_Local file",gtk_entry_new(),0,1,10));
		gtk_entry_set_icon_from_icon_name(widget->file.local,GTK_ENTRY_ICON_SECONDARY,"document-open");
		gtk_entry_set_icon_activatable(widget->file.local,GTK_ENTRY_ICON_SECONDARY,TRUE);
		gtk_entry_set_icon_tooltip_text(widget->file.local,GTK_ENTRY_ICON_SECONDARY,_("Select file"));

		// g_signal_connect(G_OBJECT(widget->file.local),"icon-press",G_CALLBACK(open_select_file_dialog),dialog);

		// Remote file name
		widget->file.remote = GTK_ENTRY(create_entry(widget,"_Remote file",gtk_entry_new(),0,2,10));



	}

	// Transfer options
	{
		GtkWidget * box = create_frame(widget, _("Transfer options"), gtk_box_new(GTK_ORIENTATION_VERTICAL,6), 0, 3, 5, 5, 8);

		for(ix=0;ix<4;ix++)
		{
			widget->options[ix] = gtk_check_button_new_with_mnemonic(gettext(ft_option[ix].label));
			gtk_widget_set_tooltip_markup(widget->options[ix],gettext(ft_option[ix].tooltip));
			// g_signal_connect(G_OBJECT(widget->options[ix]),"toggled",G_CALLBACK(option_toggled),widget);
			gtk_box_pack_start(GTK_BOX(box),widget->options[ix],FALSE,TRUE,0);
		}


	}

	gtk_widget_show_all(GTK_WIDGET(widget));

 }


 LIB3270_EXPORT GtkWidget * v3270_ft_settings_new()
 {
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270_FT_SETTINGS, NULL));
 }

