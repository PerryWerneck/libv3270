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
 #include <internals.h>
 #include "private.h"
 #include "marshal.h"
 #include <v3270/filetransfer.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270FTSettingsClass
 {
 	GtkGridClass parent_class;

 	struct
 	{
		void (*validity)(GtkWidget *, gboolean);
		void (*has_activity)(GtkWidget *, gboolean);
 	} signal;

 };

 typedef enum _invalid
 {
 	VALIDITY_TRANSFER_TYPE		= 0x0001,
	VALIDITY_LOCAL_FILENAME		= 0x0002,
	VALIDITY_REMOTE_FILENAME	= 0x0004,
 } VALIDITY_TYPE;

 struct _V3270FTSettings
 {
 	GtkGrid parent;

 	struct
 	{
 		GtkEntry * local;
 		GtkEntry * remote;
 	} file;

 	struct
 	{
		LIB3270_FT_OPTION	options;
		gboolean			is_valid;
		VALIDITY_TYPE		invalid;
 	} transfer;

	GObject				* activity;

 	GtkComboBox			* type;
	GtkWidget			* recordFormatBox;
	GtkWidget			* spaceAllocationBox;
	GtkWidget			* units;

 	GtkWidget			* options[NUM_OPTIONS_WIDGETS];
 	GtkWidget			* spins[LIB3270_FT_VALUE_COUNT];

 };

 G_DEFINE_TYPE(V3270FTSettings, V3270FTSettings, GTK_TYPE_GRID);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 enum _SIGNALS
 {
 	V3270_FT_SETTINGS_VALIDITY_SIGNAL,			///< @brief Indicates if the dialog contents is valid.
 	V3270_FT_SETTINGS_HAS_ACTIVITY_SIGNAL,		///< @brief Indicates fi the dialog has an activity.

 	V3270_FT_SETTINGS_LAST_SIGNAL
 };

 static guint v3270_ft_settings_signals[V3270_FT_SETTINGS_LAST_SIGNAL] = { 0 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270FTSettings_validity(GtkWidget G_GNUC_UNUSED(*widget), gboolean G_GNUC_UNUSED(is_valid))
 {
 	debug("%s",__FUNCTION__);
 }

static void V3270FTSettings_has_activity(GtkWidget G_GNUC_UNUSED(*widget), gboolean G_GNUC_UNUSED(is_valid))
 {
 	debug("%s",__FUNCTION__);
 }

 static void V3270FTSettings_class_init(G_GNUC_UNUSED V3270FTSettingsClass *klass)
 {
	GObjectClass * gobject_class	= G_OBJECT_CLASS(klass);

	klass->signal.validity = V3270FTSettings_validity;
	klass->signal.has_activity = V3270FTSettings_has_activity;

	v3270_ft_settings_signals[V3270_FT_SETTINGS_VALIDITY_SIGNAL] =
		g_signal_new(	"validity",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (V3270FTSettingsClass, signal.validity),
						NULL, NULL,
						v3270ft_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);


	v3270_ft_settings_signals[V3270_FT_SETTINGS_HAS_ACTIVITY_SIGNAL] =
		g_signal_new(	"has-activity",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (V3270FTSettingsClass, signal.has_activity),
						NULL, NULL,
						v3270ft_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);



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

 static GtkWidget * create_grid(GtkWidget *container, GtkAlign align)
 {
 	return v3270_box_pack_start(container,v3270_dialog_create_grid(align),TRUE,TRUE,0);
 }

static gboolean spin_format(GtkSpinButton *spin, G_GNUC_UNUSED gpointer data) {

	GtkAdjustment	* adjustment = gtk_spin_button_get_adjustment (spin);
	guint			  value = (guint) gtk_adjustment_get_value(adjustment);

	if(value < 1) {
		gtk_entry_set_text(GTK_ENTRY(spin), "");
	} else {
		g_autofree gchar * text = g_strdup_printf ("%d", value);
		gtk_entry_set_text(GTK_ENTRY(spin), text);
	}

	return TRUE;
}

 GtkWidget * create_spin_button(V3270FTSettings *widget, GtkWidget *grid, size_t row, LIB3270_FT_VALUE id)
 {
	GtkWidget * label = gtk_label_new_with_mnemonic(gettext(ft_value[id].label));
	gtk_widget_set_halign(label,GTK_ALIGN_END);

	gtk_grid_attach(GTK_GRID(grid),label,0,row,1,1);

	GtkWidget * button = gtk_spin_button_new_with_range(ft_value[id].minval,ft_value[id].maxval,1);
	// g_signal_connect(G_OBJECT(button),"value-changed",G_CALLBACK(spin_changed),dialog);
	g_signal_connect(G_OBJECT(button),"output",G_CALLBACK(spin_format),widget);

	gtk_widget_set_tooltip_markup(button,gettext(ft_value[id].tooltip));
	gtk_widget_set_tooltip_markup(label,gettext(ft_value[id].tooltip));

	gtk_label_set_mnemonic_widget(GTK_LABEL(label),button);

	gtk_grid_attach(GTK_GRID(grid),button,1,row,1,1);

	widget->spins[id] = button;

	return button;

 }

static void open_select_file_dialog(GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, GtkWidget *widget)
{
	g_autofree gchar *filename =
		v3270_select_file(
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

 	widget->transfer.options = options;

	if(options & LIB3270_FT_OPTION_RECEIVE)
	{
		debug("%s option selected","LIB3270_FT_OPTION_RECEIVE");

		gtk_widget_set_sensitive(widget->recordFormatBox,FALSE);
		gtk_widget_set_sensitive(widget->spaceAllocationBox,FALSE);

		for(ix = 0; ix < 4; ix++) {
			gtk_widget_set_sensitive(widget->spins[ix],FALSE);
			gtk_entry_set_text(GTK_ENTRY(widget->spins[ix]), "");
		}

	}
	else
	{

		debug("%s option selected","LIB3270_FT_OPTION_SEND");

		gtk_widget_set_sensitive(widget->recordFormatBox,TRUE);
		gtk_widget_set_sensitive(widget->spaceAllocationBox,TRUE);

		for(ix = 0; ix < 4; ix++) {
			gtk_widget_set_sensitive(widget->spins[ix],TRUE);
		}

	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget->options[4]),TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget->options[8]),TRUE);

	for(ix=0;ix<NUM_OPTIONS_WIDGETS;ix++) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget->options[ix]),(options & ft_option[ix].opt) == ft_option[ix].opt);
	}

	gtk_widget_set_sensitive(GTK_WIDGET(widget->file.local),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(widget->file.remote),TRUE);

 }

 static void check_for_validity_signal(V3270FTSettings *widget)
 {
 	gboolean is_valid = (widget->transfer.invalid == 0) ? TRUE : FALSE;

 	if(is_valid == widget->transfer.is_valid)
		return;

	widget->transfer.is_valid = is_valid;

	debug("Transfer is now \"%s\"", is_valid ? "valid" : "invalid");
	g_signal_emit(widget, v3270_ft_settings_signals[V3270_FT_SETTINGS_VALIDITY_SIGNAL], 0, widget->transfer.is_valid);

 }

 static void set_invalid(V3270FTSettings *widget, VALIDITY_TYPE option)
 {
 	widget->transfer.invalid |= option;
 	debug("Invalid: %08lx", (unsigned int) widget->transfer.invalid);
 	check_for_validity_signal(widget);
 }

 static void set_valid(V3270FTSettings *widget, VALIDITY_TYPE option)
 {
 	widget->transfer.invalid &= ~option;
 	debug("Invalid: %08lx", (unsigned int) widget->transfer.invalid);
 	check_for_validity_signal(widget);
 }

 static void transfer_type_changed(GtkComboBox *widget, V3270FTSettings *dialog)
 {
	gint selected = gtk_combo_box_get_active(widget);

	debug("Transfer type=%u", (unsigned int) selected);

	if(selected >= 0)
	{
		set_valid(dialog, VALIDITY_TRANSFER_TYPE);
		set_options(dialog,ft_type[selected].opt);
	}
	else
	{
		set_invalid(dialog, VALIDITY_TRANSFER_TYPE);
		gtk_widget_set_sensitive(GTK_WIDGET(dialog->file.local),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(dialog->file.remote),FALSE);
	}

 }


 static void local_file_changed(GtkEntry *entry, V3270FTSettings *widget) {

	const gchar * text = gtk_entry_get_text(entry);

	if(!text)
	{
		set_invalid(widget, VALIDITY_LOCAL_FILENAME);
		return;
	}

	if(widget->transfer.options & LIB3270_FT_OPTION_RECEIVE)
	{
		// Check for file receive options.
		g_autofree gchar * dir = g_path_get_dirname(text);

		if(*dir && !g_file_test(dir,G_FILE_TEST_IS_DIR))
		{
			debug("Folder \"%s\" is invalid",dir);
			set_invalid(widget, VALIDITY_LOCAL_FILENAME);
			return;
		}

	}
	else
	{
		// Check for file send options.
		if(!g_file_test(text,G_FILE_TEST_IS_REGULAR))
		{
			debug("File \"%s\" is invalid",text);
			set_valid(widget, VALIDITY_LOCAL_FILENAME);
			return;
		}

	}

	set_valid(widget, VALIDITY_LOCAL_FILENAME);
 }

 static void remote_file_changed(GtkEntry *entry, V3270FTSettings *widget)
 {

	const gchar * text = gtk_entry_get_text(entry);

	if(!*text)
	{
		set_invalid(widget, VALIDITY_REMOTE_FILENAME);
		return;
	}

	set_valid(widget, VALIDITY_REMOTE_FILENAME);

 }

 LIB3270_EXPORT gboolean v3270_ft_settings_set_from_filename(GtkWidget *widget, const gchar *filename)
 {
 	g_return_val_if_fail(GTK_IS_V3270_FT_SETTINGS(widget) && g_file_test(filename,G_FILE_TEST_IS_REGULAR),FALSE);

	debug("%s(%s)",__FUNCTION__,filename);

	// Setup dialog from filename.
	V3270FTSettings * settings = GTK_V3270_FT_SETTINGS(widget);
	LIB3270_FT_OPTION options = LIB3270_FT_OPTION_SEND;
	size_t ix;

	for(ix = 0; v3270_text_file_extensions[ix]; ix++)
	{
		if(g_str_has_suffix(filename,v3270_text_file_extensions[ix]))
		{
			options |= (LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_CRLF|LIB3270_FT_OPTION_REMAP);
			break;
		}
	}


	gtk_entry_set_text(settings->file.local,filename);

	g_autofree gchar * basename	= g_path_get_basename(filename);
	gtk_entry_set_text(settings->file.remote,basename);

	v3270_ft_settings_set_options(widget,options);

    return TRUE;
 }

 guint v3270_ft_settings_set_from_selection(GtkWidget *widget, GtkSelectionData *data)
 {
	gchar	**uris 	= g_strsplit((const gchar *) gtk_selection_data_get_text(data),"\n",-1);
	guint	  rc	= 0;
	size_t	  ix;

	for(ix = 0; uris[ix]; ix++)
	{
		if(!g_ascii_strncasecmp("file:///",uris[ix],8)) {

			if(v3270_ft_settings_set_from_filename(widget,uris[ix]+7))
			{
				rc++;
				break;
			}

		}
	}

	g_strfreev(uris);

	return rc;
}

 static void drag_data_received(GtkWidget *widget, GdkDragContext *context, G_GNUC_UNUSED gint x, G_GNUC_UNUSED gint y, GtkSelectionData *data, G_GNUC_UNUSED guint info, guint time)
 {
	debug("settings::%s",__FUNCTION__);
	gtk_drag_finish(context, v3270_ft_settings_set_from_selection(widget, data) > 0, FALSE, time);
 }

 static void V3270FTSettings_init(V3270FTSettings *widget)
 {
 	size_t ix;

 	// Begin with all invalid options set.
 	widget->transfer.invalid = VALIDITY_TRANSFER_TYPE|VALIDITY_LOCAL_FILENAME|VALIDITY_REMOTE_FILENAME;

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

 	// Operation type
 	{
 		GtkTreeModel	* model = GTK_TREE_MODEL(gtk_list_store_new(1,G_TYPE_STRING));
		GtkCellRenderer	* renderer = gtk_cell_renderer_text_new();

		widget->type = GTK_COMBO_BOX(create_entry(widget,"_Operation",gtk_combo_box_new_with_model(model),0,0,9));

		g_signal_connect(G_OBJECT(widget->type),"changed",G_CALLBACK(transfer_type_changed),widget);

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->type), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->type), renderer, "text", 0, NULL);

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
		g_signal_connect(G_OBJECT(widget->file.local),"changed",G_CALLBACK(local_file_changed),widget);

		// Remote file name
		widget->file.remote = GTK_ENTRY(create_entry(widget,"_Remote file",gtk_entry_new(),0,2,9));
		gtk_entry_set_max_length(widget->file.remote,PATH_MAX);
		g_signal_connect(G_OBJECT(widget->file.remote),"changed",G_CALLBACK(remote_file_changed),widget);

	}

	// Options box
	GtkWidget * options = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
	gtk_box_set_homogeneous(GTK_BOX(options),TRUE);
	g_object_set(G_OBJECT(options),"margin-top",8,NULL);
	gtk_widget_set_hexpand(options,TRUE);
	gtk_grid_attach(GTK_GRID(widget),options,0,3,10,5);

	// Transfer options
	{
		GtkWidget * box =
			v3270_box_pack_frame(
				options,
				gtk_box_new(GTK_ORIENTATION_VERTICAL,6),
				_("Transfer options"),
				NULL,
				GTK_ALIGN_START,
				FALSE,
				FALSE,
				0
			);

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

		widget->recordFormatBox =
			v3270_box_pack_frame(
				options,
				gtk_box_new(GTK_ORIENTATION_VERTICAL,6),
				_("Record format"),
				_("Specifies the record format of the data set."),
				GTK_ALIGN_CENTER,
				FALSE,
				FALSE,
				0
			);

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
		widget->spaceAllocationBox =
			v3270_box_pack_frame(
			options,
			gtk_box_new(GTK_ORIENTATION_VERTICAL,6),
			_("Space allocation units"),
			_("Specifies the units for the TSO host primary and secondary space options."),
			GTK_ALIGN_END,
			FALSE,
			FALSE,
			0
		);

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

	gtk_widget_set_sensitive(GTK_WIDGET(widget->file.local),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(widget->file.remote),FALSE);

	v3270_drag_dest_set(GTK_WIDGET(widget), G_CALLBACK(drag_data_received));

 }

 LIB3270_EXPORT GtkWidget * v3270_ft_settings_new()
 {
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270_FT_SETTINGS, NULL));
 }

 LIB3270_EXPORT void v3270_ft_settings_set_activity(GtkWidget *widget, GObject *activity)
 {
 	GTK_V3270_FT_SETTINGS(widget)->activity = activity;
 	v3270_ft_settings_reset(widget);
	g_signal_emit(widget, v3270_ft_settings_signals[V3270_FT_SETTINGS_HAS_ACTIVITY_SIGNAL], 0, (activity == NULL ? FALSE : TRUE));
 }

 LIB3270_EXPORT GObject * v3270_ft_settings_get_activity(GtkWidget *widget)
 {
 	return GTK_V3270_FT_SETTINGS(widget)->activity;
 }

 LIB3270_EXPORT void v3270_ft_settings_reset(GtkWidget *widget)
 {
 	int ix;
	V3270FTSettings * settings = GTK_V3270_FT_SETTINGS(widget);

	if(settings->activity)
	{
		gtk_entry_set_text(settings->file.local,v3270_ft_activity_get_local_filename(settings->activity));
		gtk_entry_set_text(settings->file.remote,v3270_ft_activity_get_remote_filename(settings->activity));

		v3270_ft_settings_set_options(widget,v3270_ft_activity_get_options(settings->activity));

		for(ix = 0; ix < LIB3270_FT_VALUE_COUNT; ix++)
		{
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(settings->spins[ix]), v3270_ft_activity_get_value(settings->activity,(LIB3270_FT_VALUE) ix));
		}
	}

 }

 LIB3270_EXPORT gboolean v3270_ft_settings_equals(GtkWidget *widget, GObject *activity)
 {
	V3270FTSettings * settings = GTK_V3270_FT_SETTINGS(widget);

 	if(strcmp(gtk_entry_get_text(settings->file.local),v3270_ft_activity_get_local_filename(activity)))
		return FALSE;

 	if(strcmp(gtk_entry_get_text(settings->file.remote),v3270_ft_activity_get_remote_filename(activity)))
		return FALSE;

 	return TRUE;
 }

 LIB3270_EXPORT void v3270_ft_settings_update(GtkWidget *widget)
 {
 	int ix;
	V3270FTSettings * settings = GTK_V3270_FT_SETTINGS(widget);

	debug("%s: widget=%p activity=%p",__FUNCTION__,settings,settings->activity);

	if(settings->activity)
	{
		v3270_ft_activity_set_local_filename(settings->activity,gtk_entry_get_text(settings->file.local));
		v3270_ft_activity_set_remote_filename(settings->activity,gtk_entry_get_text(settings->file.remote));

		LIB3270_FT_OPTION options = 0;

		for(ix=0;ix<NUM_OPTIONS_WIDGETS;ix++) {
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(settings->options[ix])))
				options |= ft_option[ix].opt;
		}

		v3270_ft_activity_set_options(settings->activity,options);

		for(ix = 0; ix < LIB3270_FT_VALUE_COUNT; ix++)
		{
			v3270_ft_activity_set_value(settings->activity,(LIB3270_FT_VALUE) ix, (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(settings->spins[ix])));
		}

	}
 }

 LIB3270_EXPORT GObject * v3270_ft_settings_create_activity(GtkWidget *widget)
 {
	V3270FTSettings * settings = GTK_V3270_FT_SETTINGS(widget);
	settings->activity = v3270_ft_activity_new();
	v3270_ft_settings_update(widget);
	return settings->activity;
 }

 LIB3270_EXPORT void v3270_ft_settings_set_options(GtkWidget *widget, LIB3270_FT_OPTION options)
 {
 	int ix;

	for(ix=0;ix < NUM_TYPES;ix++)
	{
		if(ft_type[ix].opt == (options & LIB3270_FT_TYPE_OPTIONS))
		{
			debug("Selecting option %s",ft_type[ix].label);
			gtk_combo_box_set_active(GTK_COMBO_BOX(GTK_V3270_FT_SETTINGS(widget)->type),ix);
			break;
		}
	}

	set_options(GTK_V3270_FT_SETTINGS(widget),options);


 }

 LIB3270_EXPORT void v3270_ft_settings_set_tso(GtkWidget *widget, gboolean tso)
 {
	V3270FTSettings * settings = GTK_V3270_FT_SETTINGS(widget);
    debug("Host is %s",(tso ? "TSO" : "not TSO"));

	GtkWidget *widgets[] =
	{
		GTK_WIDGET(settings->spins[LIB3270_FT_VALUE_PRIMSPACE]),
		GTK_WIDGET(settings->spins[LIB3270_FT_VALUE_SECSPACE]),
		GTK_WIDGET(settings->options[7]),
		GTK_WIDGET(settings->spaceAllocationBox)
	};

	size_t ix;

	for(ix = 0; ix < G_N_ELEMENTS(widgets); ix++)
		gtk_widget_set_sensitive(widgets[ix],tso);

 }
