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
  * @brief Implements the cut & paste settings widget.
  *
  */

 #include "private.h"
 #include <v3270/settings.h>
 #include <lib3270/log.h>

/*--[ Constants ]------------------------------------------------------------------------------------*/

 enum {
 	COPY_SETTINGS,
 	PASTE_SETTINGS,
 	HTML_SETTINGS,

 	GRID_COUNT
 };

 static const struct ToggleButtonDefinition toggles[] = {
 	{
 		.left = 0,
 		.top = 0,
 		.width = 1,
 		.grid = COPY_SETTINGS,
 		.id = LIB3270_TOGGLE_RECTANGLE_SELECT,
 	},

 	{
 		.left = 2,
 		.top = 0,
 		.width = 1,
 		.grid = COPY_SETTINGS,
 		.id = LIB3270_TOGGLE_MARGINED_PASTE,
 	},

 	{
 		.left = 0,
 		.top = 0,
 		.width = 1,
 		.grid = PASTE_SETTINGS,
 		.id = LIB3270_TOGGLE_SMART_PASTE,
 	}

 };

 static const struct ComboBoxDefinition combos[] = {

	{
		.grid = HTML_SETTINGS,
		.left = 0,
		.top = 0,
		.width = 1,
		.height = 1,
		.label = N_("Font"),
//		.tooltip =

		.n_columns = 2,
		.types = (const GType []) {
			G_TYPE_STRING,
			G_TYPE_UINT
		}

	},

	{
		.grid = HTML_SETTINGS,
		.left = 0,
		.top = 1,
		.width = 1,
		.height = 1,
		.label = N_("Color theme"),
//		.tooltip =

		.n_columns = 2,
		.types = (const GType []) {
			G_TYPE_STRING,
			G_TYPE_UINT
		}

	},

	{
		.grid = COPY_SETTINGS,
		.left = 0,
		.top = 1,
		.width = 1,
		.height = 1,
		.label = N_("Format"),
//		.tooltip =

		.n_columns = 2,
		.types = (const GType []) {
			G_TYPE_STRING,
			G_TYPE_UINT
		}

	},

 };

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 typedef struct _V3270ClipboardSettings {

 	V3270Settings parent;

 	struct {
		GtkToggleButton	* toggles[G_N_ELEMENTS(toggles)];	///< @brief Toggle checks.
		GtkComboBox		* combos[G_N_ELEMENTS(combos)];		///< @brief Combo-boxes.
 	} input;


 } V3270ClipboardSettings;

 typedef struct _V3270ClipboardSettingsClass {

	V3270SettingsClass parent_class;


 } V3270ClipboardSettingsClass;

 static void load(GtkWidget *w, GtkWidget *terminal);
 static void apply(GtkWidget *w, GtkWidget *terminal);

 G_DEFINE_TYPE(V3270ClipboardSettings, V3270ClipboardSettings, GTK_TYPE_V3270_SETTINGS);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void V3270ClipboardSettings_class_init(V3270ClipboardSettingsClass *klass) {

	V3270SettingsClass * widget = GTK_V3270_SETTINGS_CLASS(klass);

	widget->apply = apply;
	widget->load = load;

}

static void V3270ClipboardSettings_init(V3270ClipboardSettings *widget) {

	size_t ix;
	GtkWidget * grids[GRID_COUNT];

	// Create grids
	{
		static const gchar * labels[GRID_COUNT] = {
			N_("Copy options"),
			N_("Paste options"),
			N_("HTML options")
		};

		for(ix = 0; ix < G_N_ELEMENTS(labels); ix++) {

			grids[ix] = gtk_grid_new();

			gtk_grid_set_row_spacing(GTK_GRID(grids[ix]),6);
			gtk_grid_set_column_spacing(GTK_GRID(grids[ix]),12);

			gtk_grid_attach(
					GTK_GRID(widget),
					v3270_dialog_create_frame(grids[ix],g_dgettext(PACKAGE_NAME,labels[ix])),
					0,ix,1,1
			);

		}

	}

	v3270_settings_create_toggle_buttons(toggles, G_N_ELEMENTS(toggles), grids, widget->input.toggles);
	v3270_settings_create_combos(combos, G_N_ELEMENTS(combos), grids, widget->input.combos);

	// Setup combos
	{
		GtkCellRenderer * text_renderer	= gtk_cell_renderer_text_new();
		GtkTreeIter		  iter;
		GtkListStore	* model;

		for(ix = 0; ix < G_N_ELEMENTS(combos); ix++) {
			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->input.combos[ix]), text_renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->input.combos[ix]), text_renderer, "text", 0, NULL);
		}

		// Color scheme combo
		static const gchar * html_color_options[] = {
			N_("None (Don't export color)"),
			N_("Same of the screen")
		};

		model = GTK_LIST_STORE(gtk_combo_box_get_model(widget->input.combos[1]));
		for(ix = 0;ix < G_N_ELEMENTS(html_color_options); ix++) {

			gtk_list_store_append(model, &iter);
			gtk_list_store_set(
				model,
				&iter,
				0, g_dgettext(PACKAGE_NAME, html_color_options[ix]),
				1, (guint) ix,
				-1
			);

		}

		// Load color schemes
		{
			lib3270_autoptr(char) filename = lib3270_build_data_filename("colors.conf",NULL);
			if(g_file_test(filename,G_FILE_TEST_IS_REGULAR)) {

				GKeyFile * keyfile = g_key_file_new();

				GError * error	= NULL;
				g_key_file_load_from_file(keyfile,filename,G_KEY_FILE_NONE,&error);

				if(error) {

					g_warning(error->message);
					g_error_free(error);

				} else {

					gsize len;
					gchar **group = g_key_file_get_groups(keyfile,&len);
					GtkTreeIter	  iter;

					for(ix = 0; ix < (size_t) len; ix++) {

						g_autofree gchar * label = g_key_file_get_locale_string(keyfile,group[ix],"label",NULL,NULL);

						gtk_list_store_append((GtkListStore *) model,&iter);
						gtk_list_store_set(
							(GtkListStore *) model,
							&iter,
							0, label ? label : group[ix],
							1, (guint) 99,
							-1
						);

					}

					g_strfreev(group);

				}

				g_key_file_free(keyfile);
			}
		}

		// Copy format combo
		static const gchar * copy_formats[] = {
			N_("Plain text only"),
			N_("Complete with terminal attributes")
		};

		model = GTK_LIST_STORE(gtk_combo_box_get_model(widget->input.combos[2]));
		for(ix = 0;ix < G_N_ELEMENTS(copy_formats); ix++) {

			gtk_list_store_append(model, &iter);
			gtk_list_store_set(
				model,
				&iter,
				0, g_dgettext(PACKAGE_NAME, copy_formats[ix]),
				1, (guint) ix,
				-1
			);

		}



	}

}

GtkWidget * v3270_clipboard_settings_new() {

 	V3270Settings * settings = GTK_V3270_SETTINGS(g_object_new(V3270ClipboardSettings_get_type(), NULL));

 	settings->title = _("Cut & Paste settings");
 	settings->label = _("Clipboard");

 	return GTK_WIDGET(settings);
}

static void load(GtkWidget *w, GtkWidget *terminal) {

	size_t ix;

	V3270ClipboardSettings *widget = (V3270ClipboardSettings *) w;

	v3270_settings_load_toggle_buttons(toggles, G_N_ELEMENTS(toggles), terminal, widget->input.toggles);

	// HTML Font combo
	{
		static const gchar * html_font_options[] = {
			N_("None (Don't export font name)"),
			N_("Same of the screen")
		};

		GtkListStore * model = GTK_LIST_STORE(gtk_combo_box_get_model(widget->input.combos[0]));
		GtkTreeIter iter;
		gtk_list_store_clear(model);
		for(ix = 0;ix < G_N_ELEMENTS(html_font_options); ix++) {

			gtk_list_store_append(model, &iter);
			gtk_list_store_set(
				model,
				&iter,
				0, g_dgettext(PACKAGE_NAME, html_font_options[ix]),
				1, (guint) ix,
				-1
			);

		}

		PangoFontFamily **families;
		gint n_families;
		pango_context_list_families(gtk_widget_get_pango_context(terminal),&families, &n_families);

		for(ix=0; ((gint) ix) < n_families; ix++) {

			if(!pango_font_family_is_monospace(families[ix]))
				continue;

			const gchar *name = pango_font_family_get_name (families[ix]);
			GtkTreeIter iter;

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,0, name, 1, (guint) 99, -1);

		}

		g_free(families);

	}

}

static void apply(GtkWidget *w, GtkWidget *terminal) {

	V3270ClipboardSettings *widget = (V3270ClipboardSettings *) w;

	v3270_settings_apply_toggle_buttons(toggles,G_N_ELEMENTS(toggles),terminal,widget->input.toggles);

}


