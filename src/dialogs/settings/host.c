/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
  * @brief Implements the host settings widget.
  *
  */

 #include "private.h"
 #include <hostselect.h>
 #include <v3270/dialogs.h>
 #include <lib3270/log.h>
 #include <lib3270/toggle.h>
 #include <lib3270/properties.h>
 #include <v3270/settings/url.h>
 #include <v3270/tools.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 enum {
	CONNECTION,
	EMULATION,

	GRID_COUNT
 };

 static const struct ToggleButtonDefinition toggleList[] =
 {
 	{
 		.left = 2,
 		.top = 3,
 		.width = 2,
 		.grid = CONNECTION,
 		.id = LIB3270_TOGGLE_CONNECT_ON_STARTUP,
 	},
 	{
 		.left = 4,
 		.top = 3,
 		.width = 1,
 		.grid = CONNECTION,
		.id = LIB3270_TOGGLE_RECONNECT,
 	},
 	{
 		.left = 5,
 		.top = 3,
 		.width = 1,
 		.grid = CONNECTION,
		.id = LIB3270_TOGGLE_KEEP_ALIVE,
 	},

 	{
 		.left = 6,
 		.top = 1,
 		.width = 2,
 		.grid = EMULATION,
		.id = LIB3270_TOGGLE_ALTSCREEN,
 	},

 	/*
 	{
 		.left = 1,
 		.top = 3,
 		.width = 1,
 		.grid = EMULATION,
		.id = LIB3270_TOGGLE_MONOCASE,
 	},
 	*/

 };

 enum _entry
 {
 	ENTRY_OVERSIZE,
 	ENTRY_REMAP_FILE,
 	ENTRY_LU_NAMES,

 	ENTRY_COUNT
 };

 /// @brief Combo box descriptors.
 static const struct Combos
 {
 	ENTRY_FIELD_HEAD

	const unsigned int * values;
 	const gchar **labels;

 	unsigned int (*get)(const H3270 *);
 	int (*set)(H3270 *, unsigned int);

 } combos[] =
 {
 	{
 		.top = 0,
		.left = 0,
		.width = 2,
		.height = 1,

		.label = N_("System _type"),
		.get = (unsigned int (*)(const H3270 *)) lib3270_get_host_type,
		.set = (int (*)(H3270 *, unsigned int)) lib3270_set_host_type,

		.values = (const unsigned int [])
		{
			LIB3270_HOST_S390,
			LIB3270_HOST_AS400,
			LIB3270_HOST_TSO,
			0,
		},

		.labels = (const gchar *[])
		{
			N_( "IBM S/390"			),
			N_( "IBM AS/400"		),
			N_( "Other (TSO)" 		),
			N_( "Other (VM/CMS)"    ),
			NULL
		}

 	},
 	{
 		.top = 0,
		.left = 3,
		.width = 2,
		.height = 1,

		.label = N_("_Color table"),
		.get = lib3270_get_color_type,
		.set = lib3270_set_color_type,

		.values = (const unsigned int [])
		{
			0,
			2,
			8,
			16,
		},

		.labels = (const gchar *[])
		{
			N_( "System default"),
			N_( "Monochrome" ),
			N_( "8 colors"	 ),
			N_( "16 colors"  ),
			NULL
		}

 	},
 	{
		.top = 1,
		.left = 0,
		.width = 2,
		.height = 1,

		.label = N_("_Model"),
		.tooltip = N_("The model of 3270 display to be emulated"),

		.get = lib3270_get_model_number,
		.set = lib3270_set_model_number,

		.values = (const unsigned int [])
		{
			2,
			3,
			4,
			5
		},

		.labels = (const gchar *[])
		{
			N_( "Model 2 - 80x24"	),
			N_( "Model 3 - 80x32"	),
			N_( "Model 4 - 80x43"	),
			N_( "Model 5 - 132x27"	),
			NULL
		}

 	}
 };

 static const struct EntryFieldDefinition entryfields[] = {
 	{
		.top = 1,
		.left = 3,
		.width = 2,
		.height = 1,
 		.grid = EMULATION,

 		.label = N_( "Oversize" ),
		.tooltip = N_("Makes the screen larger than the default for the chosen model number."),
		.max_length = 7,
		.width_chars = 8,
 	},

 	{
		.top = 2,
		.left = 3,
 		.width = 4,
 		.height = 1,
 		.grid = EMULATION,

 		.label = N_( "Custom Remap" ),
		.tooltip = N_("Path to XML file with custom charset mapping."),
		.max_length = 0xFF,
		.width_chars = 50,
 	},

 	{
 		.left = 0,
 		.top = 2,
 		.width = 5,
 		.height = 1,
 		.grid = CONNECTION,

 		.label = N_( "L_U Names" ),
		.tooltip = N_("Comma separated list of LU names."),
		.max_length = 0xFF,
		.width_chars = 50,
 	},

 };

 struct _V3270HostSelectWidget
 {
 	V3270Settings parent;

	struct
	{
		GtkEntry			* entry[G_N_ELEMENTS(entryfields)];		///< @brief Entry fields for host & service name.
		GtkComboBox			* combos[G_N_ELEMENTS(combos)];			///< @brief Combo-boxes.
		GtkComboBox			* charset;								///< @brief Charset combo box.
		GtkToggleButton		* toggles[G_N_ELEMENTS(toggleList)];	///< @brief Toggle checks.
		GtkSpinButton		* auto_disconnect;						///< @brief Auto disconnect.
		GtkSpinButton		* unlock_delay;							///< @brief Unlock delay.
		GtkWidget			* url;									///< @brief The URL entry widget.

	} input;

 };

 struct _V3270HostSelectWidgetClass
 {
	V3270SettingsClass parent_class;
 };

 static void load(GtkWidget *w, GtkWidget *terminal);
 static void apply(GtkWidget *w, GtkWidget *terminal);

 G_DEFINE_TYPE(V3270HostSelectWidget, V3270HostSelectWidget, GTK_TYPE_V3270_SETTINGS);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void update_message(GtkWidget *widget, GtkWidget *terminal)
{
	gtk_widget_set_sensitive(widget, lib3270_is_disconnected(v3270_get_session(terminal)));
}

static void V3270HostSelectWidget_class_init(V3270HostSelectWidgetClass *klass)
{
	V3270SettingsClass * widget = GTK_V3270_SETTINGS_CLASS(klass);

	widget->apply = apply;
	widget->load = load;
	widget->update_message = update_message;

}

static gboolean oversize_validate(GtkEditable *editable) {

	const gchar * chars = gtk_editable_get_chars(editable,0,-1);

	if(*chars) {
        char		  junk;
        unsigned int ovc = 0, ovr = 0;

		if(sscanf(chars, "%ux%u%c", &ovc, &ovr, &junk) != 2)
			return FALSE;

		if( (ovc * ovr) > 0x4000)
			return FALSE;

	}

	return TRUE;
}

static void update_valid(V3270HostSelectWidget *settings, gboolean valid) {

    if(valid) {
		valid = v3270_url_edit_is_valid(settings->input.url);
    }

    if(valid) {
        valid = oversize_validate(GTK_EDITABLE(settings->input.entry[ENTRY_OVERSIZE]));
    }

	v3270_settings_set_valid(GTK_WIDGET(settings),valid);

}

static void url_valid(GtkWidget G_GNUC_UNUSED(*url), gboolean valid, V3270HostSelectWidget *settings) {

	if(valid) {
		update_valid(settings,valid);
		return;
	}

	v3270_settings_set_valid(GTK_WIDGET(settings),valid);

}

static void oversize_changed(GtkEditable *editable, GtkWidget *settings) {
    update_valid(GTK_V3270HostSelectWidget(settings), oversize_validate(editable));
}

static void remap_file_changed(GtkEditable *editable, V3270HostSelectWidget *settings)
{
	const gchar * filename = gtk_editable_get_chars(editable,0,-1);

	debug("%s(%s)",__FUNCTION__,filename);
	if(*filename)
	{
		v3270_settings_set_valid(GTK_WIDGET(settings),g_file_test(filename,G_FILE_TEST_IS_REGULAR));
		gtk_widget_set_sensitive(GTK_WIDGET(settings->input.charset),FALSE);
	}
	else
	{
		v3270_settings_set_valid(GTK_WIDGET(settings),TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(settings->input.charset),TRUE);
	}

}

static void select_remap_file(GtkEditable *editable, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, G_GNUC_UNUSED GtkWidget *settings)
{
	GtkWidget * dialog =
		gtk_file_chooser_dialog_new(
				_( "Select custom charset mapping"),
				GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(editable))),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				_("Cancel"),	GTK_RESPONSE_CANCEL,
				_("Select"),	GTK_RESPONSE_ACCEPT,
				NULL );


	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
	g_signal_connect(G_OBJECT(dialog),"close",G_CALLBACK(v3270_dialog_close),NULL);

	// Setup file filter
	GtkFileFilter * filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter,_("TN3270 Custom charset"));
	gtk_file_filter_add_pattern(filter,"*.xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);

	// Get current file name.
	const gchar * filename = gtk_editable_get_chars(editable,0,-1);

	if(filename && *filename)
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),filename);
	}
	else
	{
		lib3270_autoptr(char) folder = lib3270_build_data_filename("remap",NULL);

		if(folder && *folder)
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder);

	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		gtk_entry_set_text(GTK_ENTRY(editable),gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

	gtk_widget_destroy(dialog);

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

static void V3270HostSelectWidget_init(V3270HostSelectWidget *widget)
{
	GtkWidget *grids[GRID_COUNT];

	grids[CONNECTION] = widget->input.url = v3270_url_edit_new();
	g_signal_connect(grids[CONNECTION],"valid",G_CALLBACK(url_valid),widget);

	grids[EMULATION] = gtk_grid_new();

	// Connection properties
	gtk_grid_attach(
			GTK_GRID(widget),
			v3270_dialog_section_new(_("Connection"),_("Network connection settings"),grids[CONNECTION]),
			0,0,10,5
	);

	// Emulation properties
 	gtk_grid_set_row_spacing(GTK_GRID(grids[EMULATION]),6);
 	gtk_grid_set_column_spacing(GTK_GRID(grids[EMULATION]),12);

	gtk_grid_attach(
			GTK_GRID(widget),
			v3270_dialog_section_new(_("Emulation"),_("TN3270 Emulation settings"),grids[EMULATION]),
			0,6,10,5
	);

	// Entry fields
	{
		v3270_settings_create_entry_fields(entryfields, G_N_ELEMENTS(entryfields), grids, widget->input.entry);

		gtk_widget_set_hexpand(GTK_WIDGET(widget->input.entry[ENTRY_REMAP_FILE]),TRUE);

		gtk_entry_set_icon_from_icon_name(widget->input.entry[ENTRY_REMAP_FILE],GTK_ENTRY_ICON_SECONDARY,"document-open");
		gtk_entry_set_icon_activatable(widget->input.entry[ENTRY_REMAP_FILE],GTK_ENTRY_ICON_SECONDARY,TRUE);
		gtk_entry_set_icon_tooltip_text(widget->input.entry[ENTRY_REMAP_FILE],GTK_ENTRY_ICON_SECONDARY,_("Select charset remap file"));

		g_signal_connect(G_OBJECT(widget->input.entry[ENTRY_REMAP_FILE]),"icon-press",G_CALLBACK(select_remap_file),widget);
		g_signal_connect(G_OBJECT(widget->input.entry[ENTRY_REMAP_FILE]),"changed",G_CALLBACK(remap_file_changed),widget);

		g_signal_connect(G_OBJECT(widget->input.entry[ENTRY_OVERSIZE]),"changed",G_CALLBACK(oversize_changed),widget);

	}

	// Auto disconnect
	{
		GtkWidget *label = gtk_label_new_with_mnemonic(_("Auto _disconnect"));
		gtk_widget_set_halign(label,GTK_ALIGN_END);

		widget->input.auto_disconnect = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0,60,1));
		gtk_widget_set_tooltip_markup(GTK_WIDGET(widget->input.auto_disconnect),_("IDLE minutes for automatic disconnection"));
		gtk_label_set_mnemonic_widget(GTK_LABEL(label),GTK_WIDGET(widget->input.auto_disconnect));

		gtk_spin_button_set_increments(widget->input.auto_disconnect,1,1);

		gtk_grid_attach(GTK_GRID(grids[CONNECTION]),label,0,3,1,1);
		gtk_grid_attach(GTK_GRID(grids[CONNECTION]),GTK_WIDGET(widget->input.auto_disconnect),1,3,1,1);
		g_signal_connect(G_OBJECT(widget->input.auto_disconnect),"output",G_CALLBACK(spin_format),widget);
	}

	// Unlock delay
	{
		const LIB3270_UINT_PROPERTY * property = lib3270_unsigned_property_get_by_name("unlock_delay");

		GtkWidget *label = gtk_label_new_with_mnemonic(lib3270_property_get_label((const LIB3270_PROPERTY *) property));
		gtk_widget_set_halign(label,GTK_ALIGN_END);

		widget->input.unlock_delay = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(property->min,property->max,1));
		gtk_widget_set_tooltip_markup(GTK_WIDGET(widget->input.unlock_delay),lib3270_property_get_description((const LIB3270_PROPERTY *) property));
		gtk_label_set_mnemonic_widget(GTK_LABEL(label),GTK_WIDGET(widget->input.unlock_delay));

		gtk_spin_button_set_increments(widget->input.unlock_delay,1,1);

		gtk_grid_attach(GTK_GRID(grids[EMULATION]),label,6,0,1,1);
		gtk_grid_attach(GTK_GRID(grids[EMULATION]),GTK_WIDGET(widget->input.unlock_delay),7,0,1,1);
		g_signal_connect(G_OBJECT(widget->input.unlock_delay),"output",G_CALLBACK(spin_format),widget);

	}

	// Toggle checkboxes
	v3270_settings_create_toggle_buttons(toggleList, G_N_ELEMENTS(toggleList), grids, widget->input.toggles);

	// Create combo boxes
	{
		// Cell renderer
		size_t combo;
		size_t item;

		GtkCellRenderer * text_renderer	= gtk_cell_renderer_text_new();

		for(combo = 0; combo < G_N_ELEMENTS(combos); combo++) {

			GtkTreeModel * model = (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_UINT);

			widget->input.combos[combo] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

			if(combos[combo].tooltip)
				gtk_widget_set_tooltip_markup(GTK_WIDGET(widget->input.combos[combo]),combos[combo].tooltip);

			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->input.combos[combo]), text_renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->input.combos[combo]), text_renderer, "text", 0, NULL);

			v3270_grid_attach(GTK_GRID(grids[EMULATION]), (struct v3270_entry_field *) & combos[combo], GTK_WIDGET(widget->input.combos[combo]));

			for(item = 0; combos[combo].labels[item]; item++) {
				GtkTreeIter iter;
				gtk_list_store_append((GtkListStore *) model, &iter);
				gtk_list_store_set((GtkListStore *) model, &iter, 0, g_dgettext(GETTEXT_PACKAGE, combos[combo].labels[item]), 1, combos[combo].values[item], -1);
			}

		}

		// Create Charset Combo
		{
			GtkTreeModel * model = (GtkTreeModel *) gtk_list_store_new(1,G_TYPE_STRING);

			widget->input.charset = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->input.charset), text_renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->input.charset), text_renderer, "text", 0, NULL);

			static const gchar * charsets[] =
			{
				"us",
				"bracket",
				"cp500"
			};

			size_t charset;
			for(charset = 0; charset < G_N_ELEMENTS(charsets); charset++)
			{
				GtkTreeIter iter;
				gtk_list_store_append((GtkListStore *) model, &iter);
				gtk_list_store_set((GtkListStore *) model, &iter, 0, charsets[charset], -1);
			};

			static const struct v3270_entry_field descriptor =
			{
				.top = 2,
				.left = 0,
				.width = 2,
				.height = 1,

				.label = N_("_Charset"),
				.tooltip = N_("The EBCDIC host character set. "),

			};

			v3270_grid_attach(GTK_GRID(grids[EMULATION]), &descriptor, GTK_WIDGET(widget->input.charset));

		}

	}

	gtk_widget_show_all(GTK_WIDGET(widget));

}

LIB3270_EXPORT GtkWidget * v3270_host_settings_new()
{
 	V3270Settings * settings = GTK_V3270_SETTINGS(g_object_new(GTK_TYPE_V3270HostSelectWidget, NULL));

 	settings->title = _("Host settings");
 	settings->label = _("Host");

 	return GTK_WIDGET(settings);
}

LIB3270_EXPORT GtkWidget * v3270_host_select_new()
{
	return v3270_host_settings_new();
}

/*
LIB3270_EXPORT void v3270_select_host(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GtkWidget * dialog = v3270_settings_dialog_new();
	GtkWidget * settings = v3270_host_settings_new();

	v3270_settings_dialog_set_has_subtitle(dialog,FALSE);

	gtk_window_set_title(GTK_WINDOW(dialog), v3270_settings_get_title(settings));
	gtk_container_add(GTK_CONTAINER(dialog), settings);

	gtk_dialog_set_toplevel(dialog,widget);
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, widget);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 150);
	gtk_widget_show_all(dialog);

	gboolean again = TRUE;
 	while(again)
 	{
 		gtk_widget_set_sensitive(dialog,TRUE);
		gtk_widget_set_visible(dialog,TRUE);

 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_APPLY:
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_APPLY");
			v3270_settings_dialog_apply(dialog);
			again = lib3270_reconnect(v3270_get_session(widget),0);
			break;

		case GTK_RESPONSE_CANCEL:
			again = FALSE;
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_CANCEL");
			v3270_settings_dialog_revert(dialog);
			break;
 		}
 	}

	gtk_widget_destroy(dialog);

}
*/

static void apply(GtkWidget *w, GtkWidget *terminal)
{
	debug("V3270HostSelectWidget::%s",__FUNCTION__);

	V3270HostSelectWidget *widget = GTK_V3270HostSelectWidget(w);
	H3270 *hSession = v3270_get_session(terminal);

	// Apply URL
	lib3270_set_url(hSession,v3270_url_edit_get_url(widget->input.url));

	// Apply LU names
	v3270_set_lunames(terminal,gtk_entry_get_text(widget->input.entry[ENTRY_LU_NAMES]));

	// Apply combos.
	size_t combo;
	for(combo = 0; combo < G_N_ELEMENTS(combos); combo++)
	{
		GtkTreeIter	iter;

		if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget->input.combos[combo]), &iter))
		{
			GValue value = { 0, };
			gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget->input.combos[combo])),&iter,1,&value);

			combos[combo].set(hSession,g_value_get_uint(&value));

			g_value_unset(&value);

		}
	}

	// Apply charset
	{
		GtkTreeIter	iter;

		if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget->input.charset), &iter))
		{
			GValue value = { 0, };
			gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget->input.charset)),&iter,0,&value);

			lib3270_set_host_charset(hSession,g_value_get_string(&value));

			g_value_unset(&value);

		}

		v3270_set_remap_filename(terminal, gtk_entry_get_text(widget->input.entry[ENTRY_REMAP_FILE]));

	}

	// Apply toggles
	v3270_settings_apply_toggle_buttons(toggleList,G_N_ELEMENTS(toggleList),terminal,widget->input.toggles);

	// Apply oversize
	lib3270_set_oversize(hSession,gtk_entry_get_text(widget->input.entry[ENTRY_OVERSIZE]));

	// Apply auto-disconnect
	v3270_set_auto_disconnect(terminal,gtk_spin_button_get_value_as_int(widget->input.auto_disconnect));

	// Apply unlock-delay
	lib3270_set_unlock_delay(hSession, (unsigned int) gtk_spin_button_get_value(widget->input.unlock_delay));

}

static void load(GtkWidget *w, GtkWidget *terminal)
{
	debug("V3270HostSelectWidget::%s",__FUNCTION__);

	H3270 *hSession = v3270_get_session(terminal);
	V3270HostSelectWidget *widget = GTK_V3270HostSelectWidget(w);

	v3270_url_edit_set_url(widget->input.url,lib3270_get_url(hSession));

    // Load LU names
    g_autofree gchar * lunames = v3270_get_lunames(terminal);
    gtk_entry_set_text(widget->input.entry[ENTRY_LU_NAMES],lunames ? lunames : "");

    // Load unsigned int combos
	size_t combo;
	for(combo = 0; combo < G_N_ELEMENTS(combos); combo++)
	{

		GtkTreeModel	* model = gtk_combo_box_get_model(widget->input.combos[combo]);
		GtkTreeIter		  iter;
		unsigned int	  value = combos[combo].get(hSession);

		if(gtk_tree_model_get_iter_first(model,&iter))
		{
			do
			{
				GValue gVal = { 0, };
				gtk_tree_model_get_value(model,&iter,1,&gVal);
				guint iVal = g_value_get_uint(&gVal);
				g_value_unset(&gVal);

				if(iVal == value)
				{
					gtk_combo_box_set_active_iter(widget->input.combos[combo],&iter);
					break;
				}

			} while(gtk_tree_model_iter_next(model,&iter));


		}

	}

	// Load charset
	{
		const char * charset = lib3270_get_host_charset(hSession);

		if(charset)
		{
			GtkTreeModel	* model = gtk_combo_box_get_model(widget->input.charset);
			GtkTreeIter		  iter;

			if(gtk_tree_model_get_iter_first(model,&iter))
			{
				do
				{
					GValue gVal = { 0, };
					gtk_tree_model_get_value(model,&iter,0,&gVal);

					if(!g_ascii_strcasecmp(charset,g_value_get_string(&gVal)))
					{
						gtk_combo_box_set_active_iter(widget->input.charset,&iter);
						g_value_unset(&gVal);
						break;
					}

					g_value_unset(&gVal);

				} while(gtk_tree_model_iter_next(model,&iter));


			}

		}

		const gchar * remap_file = v3270_get_remap_filename(terminal);
		gtk_entry_set_text(widget->input.entry[ENTRY_REMAP_FILE],remap_file ? remap_file : "");


	}

	// Load toggles
	v3270_settings_load_toggle_buttons(toggleList, G_N_ELEMENTS(toggleList), terminal, widget->input.toggles);

	// Load oversize
	const char * oversize = lib3270_get_oversize(hSession);
	gtk_entry_set_text(widget->input.entry[ENTRY_OVERSIZE],oversize ? oversize : "");

	// Load auto disconnect
	gtk_spin_button_set_value(widget->input.auto_disconnect, v3270_get_auto_disconnect(terminal));

	// Load unlock delay
	gtk_spin_button_set_value(widget->input.unlock_delay, lib3270_get_unlock_delay(hSession));

	update_valid(widget,TRUE);

}

