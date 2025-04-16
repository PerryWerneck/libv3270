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

 #include "private.h"
 #include <lib3270/properties.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void v3270_settings_create_toggle_buttons(const struct ToggleButtonDefinition * definitions, size_t length, GtkWidget **grids, GtkToggleButton **toggles) {

	size_t toggle;

	for(toggle = 0; toggle < length; toggle++) {

		const LIB3270_TOGGLE * descriptor = lib3270_toggle_get_from_id(definitions[toggle].id);

		if(descriptor) {

			toggles[toggle] = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label(lib3270_toggle_get_label(descriptor)));

			const gchar *tooltip = lib3270_property_get_tooltip((const LIB3270_PROPERTY *) descriptor);

			if(tooltip && *tooltip)
				gtk_widget_set_tooltip_text(GTK_WIDGET(toggles[toggle]),tooltip);

			gtk_widget_set_halign(GTK_WIDGET(toggles[toggle]),GTK_ALIGN_START);
			gtk_grid_attach(GTK_GRID(grids[definitions[toggle].grid]),GTK_WIDGET(toggles[toggle]),definitions[toggle].left,definitions[toggle].top,definitions[toggle].width,1);

		}

	}

 }

 void v3270_settings_load_toggle_buttons(const struct ToggleButtonDefinition * definitions, size_t length, GtkWidget *terminal, GtkToggleButton **toggles) {

	size_t toggle;

	for(toggle = 0; toggle < length; toggle++) {

		if(toggles[toggle])
			gtk_toggle_button_set_active(toggles[toggle],v3270_get_toggle(terminal,definitions[toggle].id));

	}

 }

 void v3270_settings_apply_toggle_buttons(const struct ToggleButtonDefinition * definitions, size_t length, GtkWidget *terminal, GtkToggleButton **toggles) {

 	size_t toggle;

	for(toggle = 0; toggle < length; toggle++) {

		if(toggles[toggle])
			v3270_set_toggle(terminal, definitions[toggle].id, gtk_toggle_button_get_active(toggles[toggle]));

	}

 }

 void v3270_settings_create_entry_fields(const struct EntryFieldDefinition * definitions, size_t length, GtkWidget **grids, GtkEntry **entries) {

	size_t entry;

	for(entry = 0; entry < length; entry++) {

		entries[entry] = GTK_ENTRY(gtk_entry_new());
		gtk_entry_set_max_length(entries[entry],definitions[entry].max_length);
		gtk_entry_set_width_chars(entries[entry],definitions[entry].width_chars);

		v3270_grid_attach(
			GTK_GRID(grids[definitions[entry].grid]),
			(struct v3270_entry_field *) & definitions[entry],
			GTK_WIDGET(entries[entry])
		);

	}

 }

 void v3270_settings_create_combos(const struct ComboBoxDefinition * definitions, size_t length, GtkWidget **grids, GtkComboBox **combos) {

	size_t combo;

	for(combo = 0; combo < length; combo++) {

		if(definitions[combo].n_columns) {

			GtkTreeModel * model = GTK_TREE_MODEL(gtk_list_store_newv(definitions[combo].n_columns,(GType *) definitions[combo].types));
			combos[combo] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

		} else {

			combos[combo] = GTK_COMBO_BOX(gtk_combo_box_new());

		}

		v3270_grid_attach(
			GTK_GRID(grids[definitions[combo].grid]),
			(struct v3270_entry_field *) & definitions[combo],
			GTK_WIDGET(combos[combo])
		);

	}

 }

 void v3270_settings_create_checkboxes(const struct CheckBoxDefinition * definitions, size_t length, GtkWidget **grids, GtkToggleButton **buttons) {

 	size_t checkbox;

 	for(checkbox = 0; checkbox < length; checkbox++) {

		buttons[checkbox] = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label(g_dgettext(GETTEXT_PACKAGE,definitions[checkbox].label)));

		if(definitions[checkbox].tooltip)
			gtk_widget_set_tooltip_text(GTK_WIDGET(buttons[checkbox]),g_dgettext(GETTEXT_PACKAGE,definitions[checkbox].tooltip));

		gtk_widget_set_halign(GTK_WIDGET(buttons[checkbox]),GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grids[definitions[checkbox].grid]),GTK_WIDGET(buttons[checkbox]),definitions[checkbox].left,definitions[checkbox].top,definitions[checkbox].width,1);

 	}

 }

