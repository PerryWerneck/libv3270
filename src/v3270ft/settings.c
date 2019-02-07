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

	GtkWidget * recordFormatBox;
	GtkWidget * spaceAllocationBox;

 	GtkWidget * options[NUM_OPTIONS_WIDGETS];
 	GtkWidget * spins[LIB3270_FT_VALUE_COUNT];
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

 static GtkWidget * create_frame(GtkWidget *container, const gchar *title, GtkWidget *box, GtkAlign align)
 {
	GtkFrame			* frame		= GTK_FRAME(gtk_frame_new(""));
	g_autofree gchar	* markup	= g_strdup_printf("<b>%s</b>",title);
	GtkWidget			* label		= gtk_label_new(NULL);

	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_label_set_markup(GTK_LABEL(label),markup);
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);

	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(box));
	gtk_widget_set_halign(GTK_WIDGET(frame),align);

	g_object_set(G_OBJECT(frame),"margin-top",6,NULL);

	gtk_box_pack_start(GTK_BOX(container),GTK_WIDGET(frame),TRUE,TRUE,0);

	return box;
 }

 static GtkWidget * create_grid(GtkWidget *container, GtkAlign align)
 {
 	GtkWidget * grid = gtk_grid_new();

 	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),12);

	g_object_set(G_OBJECT(grid),"margin-top",6,NULL);
	gtk_widget_set_halign(GTK_WIDGET(grid),align);
	gtk_box_pack_start(GTK_BOX(container),GTK_WIDGET(grid),TRUE,TRUE,0);

 	return grid;
 }

 GtkWidget * create_spin_button(V3270FTSettings *widget, GtkWidget *grid, size_t row, LIB3270_FT_VALUE id)
 {
	GtkWidget * label = gtk_label_new_with_mnemonic(gettext(ft_value[id].label));
	gtk_widget_set_halign(label,GTK_ALIGN_END);

	gtk_grid_attach(GTK_GRID(grid),label,0,row,1,1);

	GtkWidget * button = gtk_spin_button_new_with_range(ft_value[id].minval,ft_value[id].maxval,1);
	// g_signal_connect(G_OBJECT(button),"value-changed",G_CALLBACK(spin_changed),dialog);
	// g_signal_connect(G_OBJECT(button),"output",G_CALLBACK(spin_format),dialog);

	gtk_widget_set_tooltip_markup(button,gettext(ft_value[id].tooltip));
	gtk_widget_set_tooltip_markup(label,gettext(ft_value[id].tooltip));

	gtk_label_set_mnemonic_widget(GTK_LABEL(label),button);

	gtk_grid_attach(GTK_GRID(grid),button,1,row,1,1);

	widget->spins[id] = button;

	return button;

 }

static void open_select_file_dialog(GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, GtkWidget *widget)
{
	v3270_autofree gchar *filename =
		v3270ft_select_file(
			gtk_widget_get_toplevel(widget),
			_("Select local file"),
			_("Select"),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			gtk_entry_get_text(entry),
			N_("All files"), "*.*",
			N_("Text files"), "*.txt",
			NULL
		);

	if(filename) {
		gtk_entry_set_text(entry,filename);
	}

 }

 static void set_options(V3270FTSettings *widget, LIB3270_FT_OPTION options)
 {
 	size_t ix;

	if(options & LIB3270_FT_OPTION_RECEIVE)
	{
		debug("%s option selected","LIB3270_FT_OPTION_RECEIVE");

		gtk_widget_set_sensitive(widget->recordFormatBox,FALSE);
		gtk_widget_set_sensitive(widget->spaceAllocationBox,FALSE);

		for(ix = 0; ix < 4; ix++) {
			gtk_widget_set_sensitive(widget->spins[ix],FALSE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget->spins[ix]),0);
		}

	}
	else
	{
		debug("%s option selected","LIB3270_FT_OPTION_SEND");

		gtk_widget_set_sensitive(widget->recordFormatBox,TRUE);
		gtk_widget_set_sensitive(widget->spaceAllocationBox,TRUE);

		for(ix = 0; ix < 4; ix++) {
			gtk_widget_set_sensitive(widget->spins[ix],TRUE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget->spins[ix]),0);
		}

	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget->options[4]),TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget->options[8]),TRUE);

	for(ix=0;ix<NUM_OPTIONS_WIDGETS;ix++) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget->options[ix]),(options & ft_option[ix].opt) == ft_option[ix].opt);
	}

 }

 static void transfer_type_changed(GtkComboBox *widget, V3270FTSettings *dialog)
 {
	gint selected = gtk_combo_box_get_active(widget);

	debug("Transfer type=%u", (unsigned int) selected);

	if(selected >= 0)
		set_options(dialog,ft_type[selected].opt);

 }

 static void V3270FTSettings_init(V3270FTSettings *widget)
 {
 	size_t ix;

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

 	// Operation type
 	{
 		GtkTreeModel	* model = GTK_TREE_MODEL(gtk_list_store_new(1,G_TYPE_STRING));
		GtkWidget		* entry = create_entry(widget,"_Operation",gtk_combo_box_new_with_model(model),0,0,9);
		GtkCellRenderer	* renderer = gtk_cell_renderer_text_new();

		g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(transfer_type_changed),widget);

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(entry), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(entry), renderer, "text", 0, NULL);

		for(ix=0;ix < NUM_TYPES;ix++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(ft_type[ix].label), -1);
		}


 	}

 	// Local and remote file names.
	{
		// Local file name
		widget->file.local = GTK_ENTRY(create_entry(widget,"_Local file",gtk_entry_new(),0,1,9));
		gtk_entry_set_icon_from_icon_name(widget->file.local,GTK_ENTRY_ICON_SECONDARY,"document-open");
		gtk_entry_set_icon_activatable(widget->file.local,GTK_ENTRY_ICON_SECONDARY,TRUE);
		gtk_entry_set_icon_tooltip_text(widget->file.local,GTK_ENTRY_ICON_SECONDARY,_("Select file"));

		g_signal_connect(G_OBJECT(widget->file.local),"icon-press",G_CALLBACK(open_select_file_dialog),widget);

		// Remote file name
		widget->file.remote = GTK_ENTRY(create_entry(widget,"_Remote file",gtk_entry_new(),0,2,9));
		gtk_entry_set_max_length(widget->file.remote,PATH_MAX);

	}

	// Options box
	GtkWidget * options = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
	gtk_box_set_homogeneous(GTK_BOX(options),TRUE);
	g_object_set(G_OBJECT(options),"margin-top",8,NULL);
	gtk_widget_set_hexpand(options,TRUE);
	gtk_grid_attach(GTK_GRID(widget),options,0,3,10,5);

	// Transfer options
	{
		GtkWidget * box = create_frame(options, _("Transfer options"), gtk_box_new(GTK_ORIENTATION_VERTICAL,6),GTK_ALIGN_START);

		for(ix=0;ix<4;ix++)
		{
			widget->options[ix] = gtk_check_button_new_with_mnemonic(gettext(ft_option[ix].label));
			gtk_widget_set_tooltip_markup(widget->options[ix],gettext(ft_option[ix].tooltip));
			// g_signal_connect(G_OBJECT(widget->options[ix]),"toggled",G_CALLBACK(option_toggled),widget);
			gtk_box_pack_start(GTK_BOX(box),widget->options[ix],FALSE,TRUE,0);
		}


	}

	// Record format
	{
		GSList * group = NULL;
		widget->recordFormatBox = create_frame(options, _("Record format"), gtk_box_new(GTK_ORIENTATION_VERTICAL,6),GTK_ALIGN_CENTER);

		for(ix=4;ix<8;ix++)
		{
			widget->options[ix] = gtk_radio_button_new_with_label(group,gettext(ft_option[ix].label));
			gtk_widget_set_tooltip_markup(widget->options[ix],gettext(ft_option[ix].tooltip));
			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget->options[ix]));
			// g_signal_connect(G_OBJECT(widget->options[ix]),"toggled",G_CALLBACK(option_toggled),widget);
			gtk_box_pack_start(GTK_BOX(widget->recordFormatBox),widget->options[ix],FALSE,TRUE,0);

		}
	}

	// Space allocation units
	{
		GSList * group = NULL;
		widget->spaceAllocationBox = create_frame(options, _("Space allocation units"), gtk_box_new(GTK_ORIENTATION_VERTICAL,6),GTK_ALIGN_END);

		for(ix=8;ix<12;ix++)
		{
			widget->options[ix] = gtk_radio_button_new_with_label(group,gettext(ft_option[ix].label));
			gtk_widget_set_tooltip_markup(widget->options[ix],gettext(ft_option[ix].tooltip));
			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget->options[ix]));
			// g_signal_connect(G_OBJECT(widget->options[ix]),"toggled",G_CALLBACK(option_toggled),widget);
			gtk_box_pack_start(GTK_BOX(widget->spaceAllocationBox),widget->options[ix],FALSE,TRUE,0);

		}
	}

	// Values box
	GtkWidget * values = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
	gtk_box_set_homogeneous(GTK_BOX(values),TRUE);
	g_object_set(G_OBJECT(values),"margin-top",8,NULL);
	gtk_widget_set_hexpand(values,TRUE);
	gtk_grid_attach(GTK_GRID(widget),values,0,8,10,2);

	gtk_widget_show_all(GTK_WIDGET(widget));

	// Values
	{
		GtkWidget * box = create_grid(values,GTK_ALIGN_START);

		create_spin_button(widget, box, 0, LIB3270_FT_VALUE_LRECL);
		create_spin_button(widget, box, 1, LIB3270_FT_VALUE_BLKSIZE);

	}

	{
		GtkWidget * box = create_grid(values,GTK_ALIGN_CENTER);

		create_spin_button(widget, box, 0, LIB3270_FT_VALUE_PRIMSPACE);
		create_spin_button(widget, box, 1, LIB3270_FT_VALUE_SECSPACE);

	}

	{
		GtkWidget * box = create_grid(values,GTK_ALIGN_END);
		create_spin_button(widget, box, 0, LIB3270_FT_VALUE_DFT);

	}

 }

 LIB3270_EXPORT GtkWidget * v3270_ft_settings_new()
 {
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270_FT_SETTINGS, NULL));
 }

